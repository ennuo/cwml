#include <System.h>
#include <DebugLog.h>
#include <memory/Bucket.h>
#include <memory/Memory.h>

#include <cwml/Context.h>

#include <sys/prx.h>
#include <cell/sysmodule.h>

#include <vector.h>
#include <filepath.h>
#include <MMString.h>
#include <LoadingScreen.h>

#include <cell/fs/cell_fs_file_api.h>
#include <ReadINI.h>

cwml::GameContext* gPluginContext;
extern const char* gTitleID;
sys_prx_id_t gMyID;


const char* kAutoPatchRoot = "/dev_usb007";


CFilePath GetGeneratedPatchPath()
{
    char buf[255];
    if (FileExists(kAutoPatchRoot))
    {
        sprintf(buf, "%s/%s_patch.yml", kAutoPatchRoot, gTitleID);
        return CFilePath(buf);
    }

    sprintf(buf, "output/%s_patch.yml", gTitleID);
    return CFilePath(FPR_GAMEDATA, buf);
}

bool cwml::GameContext::LoadModule(LoadedPlugin& plugin)
{
    if (plugin.State >= ePluginState_GenericError)
    {
        MMLog("cwml: skipping load of %s since it already errored during this session\n", plugin.Path.GetFilename());
        return false;
    }

    if (plugin.Prx > 0) return true;

    plugin.State = ePluginState_Unloaded;

    // LBP2+ uses pre-allocated memory containers for all allocations, so if we just try using free memory,
    // we'll run out of memory for plugins basically immediately, so allocate inside the container instead.
// #ifndef LBP1
    // plugin.Prx = sys_prx_load_module_on_memcontainer(plugin.Path.c_str(), gSlabAlloc.BucketMem[MEM_MAIN].GetContainer(), 0ull, NULL);
// #else
    plugin.Prx = sys_prx_load_module(plugin.Path.c_str(), 0ull, NULL);
// #endif

    if (plugin.Prx < CELL_OK)
    {
        plugin.State = ePluginState_LoadModuleFailed;
        MMLog("cwml: error occurred while loading %s (%08x)\n", plugin.Path.GetFilename(), plugin.Prx);
        return false;
    }

    cwml::PluginLoad load;
    memset(&load, 0, sizeof(cwml::PluginLoad));
    load.Version = cwml::PluginLoad::kVersion;
    load.PrxId = plugin.Prx;
    load.PrxFilePath = plugin.Path.c_str();
    load.Roots[FPR_GAMEDATA] = gGameDataPath.c_str();
    load.Roots[FPR_BLURAY] = gBaseDir.c_str();
    load.Roots[FPR_SYSCACHE] = gSysCachePath.c_str();
    load.Roots[FPR_MODLOADER] = gModLoaderPath.c_str();
    load.Roots[FPR_PLUGIN] = gModLoaderPath.c_str();
    load.TitleID = gTitleID; // hardcoded for now
    load.ModAPI = this;

    int ret;
    if ((ret = sys_prx_start_module(plugin.Prx, 1, &load, &ret, 0ull, NULL)) != CELL_OK)
    {
        plugin.State = ePluginState_StartModuleFailed;
        MMLog("cwml: error occurred while starting %s (%08x)\n", plugin.Path.GetFilename(), plugin.Prx);
        return false;
    }

    if (load.VersionMismatch)
    {
        plugin.State = ePluginState_VersionMismatch;
        MMLog("cwml: failed to load %s because version is incompatible with installed loader\n", plugin.Path.GetFilename());
        return false;
    }

    plugin.Instance = load.pPlugin;
    plugin.Interface = load.pPluginInterface;

    if (plugin.Instance == NULL)
    {
        plugin.State = ePluginState_NoSource;
        MMLog("cwml: failed to load %s because no plugin class was instantiated\n", plugin.Path.GetFilename());
        return false;
    }

    const char* name = plugin.Instance->GetName();
    char fmt[512];
    sprintf(fmt, "config/%s.ini", name);
    
    CIniSettings settings;
    if (settings.ReadIniFile(CFilePath(FPR_MODLOADER, fmt)))
    {
        MMLog("cwml: found config file for %s\n", name);
        ConfigOptionBase* opt = plugin.Instance->GetConfig();
        while (opt != NULL)
        {
            const char* value = settings.GetString(opt->GeIniFilename(), NULL);
            if (value != NULL)
            {
                MMLog("cwml: refreshing %s (%s) from config\n", opt->GeIniFilename(), opt->GetDisplayName());
                opt->SetString(value);
            }
            
            opt = opt->GetNext();
        }
    }
    else MMLog("cwml: no config file for %s\n", name);

    if (!plugin.Instance->OnAttach())
    {
        plugin.State = ePluginState_AttachFailed;
        MMLog("cwml: attach failed for %s\n", plugin.Path.GetFilename());
        return false;
    }

    if (Ib::WriteCache != NULL)
        Ib::WriteCache->AddPRX(plugin.Prx);
    
    plugin.State = ePluginState_Loaded;
    
    return true;
}

void cwml::GameContext::UnloadModule(LoadedPlugin& plugin)
{
    if (plugin.Prx <= 0) return;

    if (plugin.Interface != NULL) delete plugin.Interface;

    if (plugin.Instance != NULL)
    {
        if (plugin.State != ePluginState_AttachFailed)
            plugin.Instance->OnDetach();
        
        delete plugin.Instance;
    }

    if (plugin.State != ePluginState_StartModuleFailed)
        sys_prx_stop_module(plugin.Prx, 0, NULL, NULL, 0ull, NULL);
    sys_prx_unload_module(plugin.Prx, 0ull, NULL);
    
    plugin.Prx = 0;
    plugin.Instance = NULL;
    plugin.Interface = 0;

    if (plugin.State < ePluginState_GenericError)
        plugin.State = ePluginState_Unloaded;
}

bool cwml::GameContext::OpenPlugin(LoadedPlugin& plugin, bool was_reloaded)
{
    if (plugin.State == ePluginState_Open) return true;
    if (plugin.State != ePluginState_Loaded) return false;

    MMLog("cwml: opening %s...\n", plugin.Path.GetFilename());
    plugin.State = plugin.Instance->OnOpen(was_reloaded) ? ePluginState_Open : ePluginState_OpenFailed;
    MMLog("%s\n", plugin.State == ePluginState_Open ? "OK" : "FAIL");
}

void cwml::GameContext::LoadPlugins()
{
    MMLog("cwml: loading plugins...\n");

    CFilePath plugins_root = CFilePath(FPR_MODLOADER, "plugins");
    DirHandle fd;
    if (!DirectoryOpen(plugins_root, fd))
    {
        MMLog("cwml: no plugins directory\n");
        return;
    }

    char filename[256];
    while (true)
    {
        if (!DirectoryRead(fd, filename, sizeof(filename)))
        {
            DirectoryClose(fd);
            break;
        }

        if (*filename == '.') continue;

        CFilePath fp(plugins_root);
        fp.Append(filename);

        int attr = FileAttributes(fp);
        if (attr & CELL_FS_S_IFREG)
        {
            CFilePath filename = fp.GetFilename();
            filename.StripExtension();
            if (!Config.GetBool(filename.c_str(), true))
            {
                MMLog("cwml: skipping disabled plugin %s\n", fp.GetFilename());
                continue;
            }

            MMLog("cwml: found %s\n", fp.GetFilename());

            LoadedPlugin plugin;
            plugin.Path = fp;
            FileStat(fp, plugin.LastModified, plugin.LastSize);

            Plugins.push_back(plugin);
            LoadModule(Plugins.back());
        }
    }
}

void cwml::GameContext::OpenPlugins()
{
    MMLog("cwml: opening plugins\n");
    for (LoadedPlugin* plugin = Plugins.begin(); plugin != Plugins.end(); ++plugin)
        OpenPlugin(*plugin, false);
}

void cwml::GameContext::ClosePlugins()
{
    MMLog("cwml: closing plugins\n");
    for (LoadedPlugin* plugin = Plugins.begin(); plugin != Plugins.end(); ++plugin)
    {
        if (plugin->State != ePluginState_Open) continue;
        plugin->Instance->OnClose();
    }
}

void cwml::GameContext::UnloadPlugins()
{
    MMLog("cwml: unloading plugins\n");
    for (LoadedPlugin* plugin = Plugins.begin(); plugin != Plugins.end(); ++plugin)
        UnloadModule(*plugin);
    Plugins.clear();
}

void cwml::GameContext::PerFrameUpdate()
{
    for (LoadedPlugin* plugin = Plugins.begin(); plugin != Plugins.end(); ++plugin)
    {
        if (plugin->State == ePluginState_Open) 
            plugin->Instance->OnUpdate();
        
        if (plugin->State >= ePluginState_GenericError)
            UnloadModule(*plugin);
    }

    CheckReloadFlag();
}

#ifdef CWML_HOT_RELOAD
    void ReloadThreadStatic(u64 arg)
    {
        cwml::GameContext* ctx = (cwml::GameContext*)arg;
        ctx->ReloadHandler();
        ExitPPUThread(0);
    }

    void cwml::GameContext::ReloadHandler()
    {
        while (true)
        {
            if (Terminate) return;

            int wait_time = 1000;
            for (LoadedPlugin* it = Plugins.begin(); it != Plugins.end(); ++it)
            {
                LoadedPlugin& plugin = *it;

                bool can_reload = plugin.State >= ePluginState_GenericError || plugin.State == ePluginState_Open;
                if (!can_reload) continue;

                u64 modtime, size;
                FileStat(plugin.Path, modtime, size);
                bool file_changed = size != plugin.LastSize || modtime != plugin.LastModified;

                plugin.LastSize = size;
                plugin.LastModified = modtime;

                if (plugin.WasChangedLastTick)
                {
                    // Keep waiting until whatever I/O is happening on this file is finished.
                    if (file_changed)
                    {
                        wait_time = 500;
                        continue;
                    }

                    plugin.WasChangedLastTick = false;
                    plugin.WantReload = true;
                    MMLog("cwml: marking %s for reload\n", plugin.Path.GetFilename());
                }
                else if (file_changed)
                {
                    plugin.WasChangedLastTick = true;
                    wait_time = 500;
                }
            }

            ThreadSleep(wait_time);
        }
    }

    void cwml::GameContext::StartReloadHandler()
    {
        MMLog("cwml: starting hot reload thread\n");
        ReloadThread = CreatePPUThread(&ReloadThreadStatic, (u64)this, "cwml reload handler", 1000, 0x10000, true);
    }
    
    void cwml::GameContext::CloseReloadHandler()
    {
        MMLog("cwml: stopping hot reload thread\n");

        u64 retval;

        Terminate = true;
        JoinPPUThread(ReloadThread, &retval);

        ReloadThread = 0;
    }

    void cwml::GameContext::CheckReloadFlag()
    {
        for (LoadedPlugin* it = Plugins.begin(); it != Plugins.end(); ++it)
        {
            LoadedPlugin& plugin = *it;
            if (plugin.WantReload)
            {
                plugin.WantReload = false;

                if (plugin.State == ePluginState_Open)
                {
                    plugin.Instance->OnClose();
                    plugin.State = ePluginState_Loaded;
                }

                UnloadModule(plugin);

                // clear error state
                plugin.State = ePluginState_Unloaded;

                if (LoadModule(plugin) && plugin.State == ePluginState_Loaded)
                    OpenPlugin(plugin, true);
            }
        }
    }
#else
    void cwml::GameContext::StartReloadHandler() {}
    void cwml::GameContext::CloseReloadHandler() {}
    void cwml::GameContext::CheckReloadFlag() {}

#endif

Ib_DefinePort(SetActiveLoadingScreen, void*, CTextState const* text, TextStateCallback* callback, bool allow_if_other_active_screens);
Ib_DefinePort(CancelActiveLoadingScreen, void, void* handle, bool call_callback, u32 result);


bool WarmUpModLoader()
{

    gPluginContext = new cwml::GameContext();
    if (!gPluginContext->IsEnabled())
    {
        MMLog("cwml: modloader disabled\n");
        delete gPluginContext;
        gPluginContext = NULL;
        return true;
    }

    MMLog("cwml: starting modloader...\n");

    if (Ib::WriteCache != NULL)
    {
        Ib::WriteCache->AddExecutableModule(CFilePath(FPR_GAMEDATA, "EBOOT.BIN"));
        Ib::WriteCache->AddPRX(gMyID);
    }

    gPluginContext = new cwml::GameContext();
    gPluginContext->LoadPlugins();
    return true;
}

bool OpenModLoader()
{
    MMLog("cwml: opening plugins...\n");
    if (gPluginContext == NULL) return false;

    if (Ib::NeedsRebuildPatch())
    {
        CFilePath fp = GetGeneratedPatchPath();
        bool is_auto = strstr(fp.c_str(), kAutoPatchRoot) != NULL;

        Ib::GeneratePatchYML("cwml", "LittleBigPlanet 2", gTitleID, fp.c_str());

        char msg[1024];
        if (is_auto)
        {
            strcpy(msg, "LLVM patches have been rebuilt, please re-enable the newly generated patches in the patch manager or restart the game in PPU interpreter mode!");
        }
        else
        {
            sprintf(msg, "Use of LLVM requires patches to be re-built, please copy the generated patch at %s to your patches folder or restart game in PPU interpreter mode!", fp.c_str());
        }

        CTextState state;
        MultiByteToTChar(state.title, "Patch Manager", NULL);
        MultiByteToTChar(state.text, msg, NULL);
        state.button_mode = 1;

        void* handle = SetActiveLoadingScreen(&state, NULL, true);
        ThreadSleep(2000);
        CancelActiveLoadingScreen(handle, false, 0);

        return false;
    }


    gPluginContext->OpenPlugins();
    gPluginContext->StartReloadHandler();

    return true;

}

void StopModLoader()
{
    if (gPluginContext == NULL) return;

    MMLog("cwml: stopping modloader...\n");

    gPluginContext->CloseReloadHandler();
    gPluginContext->ClosePlugins();
    gPluginContext->UnloadPlugins();
    
    delete gPluginContext;
    gPluginContext = NULL;
}

void UpdateModLoader()
{
    if (gPluginContext != NULL)
        gPluginContext->PerFrameUpdate();
}

extern bool BootCheck();
CInitStep gPreInit[] =
{
    CInitStep("cwml::dummy"),
    CInitStep("cwml::globals").SetInitFunc(BootCheck),
    CInitStep("cwml::warmup_plugins")
        .SetInitFunc(WarmUpModLoader)
        .SetCloseFunc(StopModLoader),
    CInitStep()
};

CInitStep gInit[] =
{
    CInitStep("cwml::dummy"),
    CInitStep("cwml::modloader")
        .SetInitFunc(OpenModLoader),
    CInitStep()
};

void GoLoco()
{
    gMyID = sys_prx_get_my_module_id();

    MMLog("cwml: going loco!\n");

    CInitStep* prx = FindInitStep("PRX");
    *gPreInit = *prx;
    *prx = CInitStep();
    (prx - 1)->ChainTo = gPreInit;
    gPreInit[ARRAY_LENGTH(gPreInit) - 2].ChainTo = prx + 1;

    

    CInitStep* ls = FindInitStep(gPs3Test1InitSteps, "LegalBlock");
    *gInit = *ls;
    *ls = CInitStep();
    (ls - 1)->ChainTo = gInit;
    gInit[ARRAY_LENGTH(gInit) - 2].ChainTo = ls + 1;

    // On LBP2 we need to decrease the size of the main memory pool
    // by a few megabytes so that we have space for our plugins.
    // This shouldn't affect normal gameplay(?)
#ifndef LBP1
    gBucketDefs[MEM_MAIN].Size -= (1048576 * 3);
#endif


    // Ib_PokeCall(0x0025f164, UpdateModLoader);
}