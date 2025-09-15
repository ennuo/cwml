#pragma once

#include <sys/prx.h>
#include <thread.h>
#include <cwml/Plugin.h>
#include <filepath.h>
#include <vector.h>
#include <ReadINI.h>

namespace cwml
{
    enum EPluginState
    {
        ePluginState_Unloaded = 0,
        ePluginState_Loaded = 1,
        ePluginState_Disabled = 2,
        ePluginState_Open = 3,

        ePluginState_GenericError = 1000,
        ePluginState_LoadModuleFailed = 1001,
        ePluginState_StartModuleFailed = 1002,
        ePluginState_NoSource = 1003,
        ePluginState_AttachFailed = 1004,
        ePluginState_VersionMismatch = 1005,
        ePluginState_OpenFailed = 1006
    };

    class LoadedPlugin {
    public:
        LoadedPlugin();
    public:
        CFilePath Path;
        sys_prx_id_t Prx;
        Plugin* Instance;
        PluginInterface* Interface;
        u64 LastModified;
        u64 LastSize;
        int State;
        bool WasChangedLastTick;
        bool WantReload;
    };

    class GameContext : public Interface {
    public:
        GameContext();
        ~GameContext();
    public:
        inline const bool IsEnabled()
        {
            return Config.GetBool("enabled", true);
        }

        inline const CIniSettings& GetConfig() const { return Config; }
    public:
        void LoadPlugins();
        void OpenPlugins();
        void ClosePlugins();
        void UnloadPlugins();

        void PerFrameUpdate();

        void StartReloadHandler();
        void CloseReloadHandler();
        void CheckReloadFlag();
        void ReloadHandler();

        bool LoadModule(LoadedPlugin& plugin);
        void UnloadModule(LoadedPlugin& plugin);
        bool OpenPlugin(LoadedPlugin& plugin, bool was_reloaded);


        int GetVersion() const;
        PluginInterface* GetInterface(const char*);
        void GetSharedHookArguments(Ib::InitArgs& args) const;
    private:
        CIniSettings Config;
        CVector<LoadedPlugin> Plugins;
        THREAD ReloadThread;
        bool Terminate;
    };
}
