#pragma once

#include <filepath.h>
#include <cwml/Config.h>

typedef int PluginHandle;
const PluginHandle kInvalidPlugin = -1;

namespace cwml
{
    // Plugins can optionally return a pointer to an API class instance
    // that allows other plugins to call their functions.
    class PluginInterface {
    public:
        virtual ~PluginInterface() = 0;
        virtual int GetVersion() const = 0;
    };

    class Plugin {
    public:
        virtual ~Plugin() {}
        virtual const char* GetName() const = 0;

        // The function that gets called when the modloader attaches this plugin to the game.
        virtual bool OnAttach() {}

        // The function that gets called when the modloader detaches this plugin from the game.
        virtual void OnDetach() {}
        
        // The function that gets called by the game's initialization steps
        virtual bool OnOpen(bool was_reloaded) { return true; }

        // The function that gets called by the game's close steps
        virtual void OnClose() {}

        // This function gets called once per frame
        virtual void OnUpdate() {}

        inline virtual ConfigOptionBase* GetConfig() const
        {
            return gConfigOptionHead;
        }
    };

    class Interface {
    public:
        enum { kVersion = 1 };
    public:
        virtual int GetVersion() const = 0;
        virtual void GetSharedHookArguments(Ib::InitArgs& args) const = 0;
        virtual PluginInterface* GetInterface(const char*) = 0;
    };
    
    // Structure passed to plugins on load
    class PluginLoad {
    public:
        enum { kVersion = 1 };
    public:
        int Version;
        
        sys_prx_id_t PrxId;

        const char* PrxFilePath;
        const char* Roots[FPR_MAX];
        const char* TitleID;

        bool VersionMismatch;
        Interface* ModAPI;

        Plugin* pPlugin;
        PluginInterface* pPluginInterface;
    };

    extern Interface* api;
};

#define PLUGIN_PRX_BOILERPLATE(plugin_name) \
    extern "C" int cwml_load(size_t args, void* argp); \
    extern "C" int cwml_unload(size_t args, void* argp); \
    SYS_MODULE_INFO(plugin_name, 0, 1, 0); \
    SYS_MODULE_START(cwml_load); \
    SYS_MODULE_STOP(cwml_unload); \
    extern "C" int __cxa_pure_virtual() { return 0; } \
    cwml::Interface* cwml::api; \
    typedef void (*func_ptr) (void); \
    extern func_ptr __CTOR_LIST__[]; \
    extern func_ptr __CTOR_END__[]; \
    extern func_ptr __DTOR_LIST__[]; \
    extern func_ptr __DTOR_END__[];

#define PLUGIN_STATIC_CONSTRUCTORS \
    __SIZE_TYPE__ nptrs = ((__SIZE_TYPE__)__CTOR_END__ - (__SIZE_TYPE__)__CTOR_LIST__) / sizeof(__SIZE_TYPE__); \
    for (unsigned i = 0; i < nptrs; ++i) \
        __CTOR_LIST__[i]();

#define PLUGIN_STATIC_DESTRUCTORS \
    __SIZE_TYPE__ nptrs = ((__SIZE_TYPE__)__DTOR_END__ - (__SIZE_TYPE__)__DTOR_LIST__) / sizeof(__SIZE_TYPE__); \
    for (unsigned i = 0; i < nptrs; ++i) \
        __DTOR_LIST__[i]();

#define PLUGIN_VERSION_CHECK \
    if (load->Version != cwml::PluginLoad::kVersion) \
    { \
        load->VersionMismatch = true; \
        return SYS_PRX_START_OK; \
    }

#define PLUGIN_LOAD_FILE_PATH_ROOTS \
    gGameDataPath = load->Roots[FPR_GAMEDATA]; \
    gBaseDir = load->Roots[FPR_BLURAY]; \
    gSysCachePath = load->Roots[FPR_SYSCACHE]; \
    gModLoaderPath = load->Roots[FPR_MODLOADER]; \
    gPluginPath = load->Roots[FPR_PLUGIN];

#define PLUGIN_INITIALIZE_IB \
    Ib::InitArgs ib_args; \
    cwml::api->GetSharedHookArguments(ib_args); \
    Ib::Initialize(&ib_args);

#define PLUGIN_INITIALIZE_CONFIG \
    cwml::ConfigOptionBase* cwml::gConfigOptionHead;

#define REGISTER_PLUGIN_AND_API(plugin_name, plugin_api_name) \
    PLUGIN_PRX_BOILERPLATE(plugin_name) \
    PLUGIN_INITIALIZE_CONFIG \
    extern "C" int cwml_load(size_t, void* argp) \
    { \
        PLUGIN_STATIC_CONSTRUCTORS \
        cwml::PluginLoad* load = (cwml::PluginLoad*)argp; \
        cwml::api = load->ModAPI; \
        PLUGIN_VERSION_CHECK \
        PLUGIN_LOAD_FILE_PATH_ROOTS \
        PLUGIN_INITIALIZE_IB \
        load->pPlugin = new plugin_name(); \
        load->pPluginInterface = new plugin_api_name(); \
        return SYS_PRX_START_OK; \
    } \
    extern "C" int cwml_unload(size_t, void* argp) \
    { \
        PLUGIN_STATIC_DESTRUCTORS \
        return SYS_PRX_STOP_OK; \
    }

#define REGISTER_PLUGIN(plugin_name) \
    PLUGIN_PRX_BOILERPLATE(plugin_name) \
    PLUGIN_INITIALIZE_CONFIG \
    extern "C" int cwml_load(size_t, void* argp) \
    { \
        PLUGIN_STATIC_CONSTRUCTORS \
        cwml::PluginLoad* load = (cwml::PluginLoad*)argp; \
        cwml::api = load->ModAPI; \
        PLUGIN_VERSION_CHECK \
        PLUGIN_LOAD_FILE_PATH_ROOTS \
        PLUGIN_INITIALIZE_IB \
        load->pPlugin = new plugin_name(); \
        return SYS_PRX_START_OK; \
    } \
    extern "C" int cwml_unload(size_t, void* argp) \
    { \
        PLUGIN_STATIC_DESTRUCTORS \
        return SYS_PRX_STOP_OK; \
    }
