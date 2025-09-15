#include <cwml/Context.h>

using namespace cwml;

GameContext::GameContext() : Plugins(), Config(), ReloadThread(), Terminate()
{
    Config.ReadIniFile(CFilePath(FPR_MODLOADER, "config.ini"));
}

GameContext::~GameContext()
{

}

int GameContext::GetVersion() const
{
    return kVersion;
}

PluginInterface* GameContext::GetInterface(const char* plugin_name)
{
    for (LoadedPlugin* plugin = Plugins.begin(); plugin != Plugins.end(); ++plugin)
    {
        if (plugin->Instance == NULL) continue;
        if (strcmp(plugin_name, plugin->Instance->GetName()) == 0)
            return plugin->Interface;
    }
}

LoadedPlugin::LoadedPlugin() : Path(), Prx(), Instance(), 
LastModified(), LastSize(), WantReload(), Interface(), State(), WasChangedLastTick()
{

}