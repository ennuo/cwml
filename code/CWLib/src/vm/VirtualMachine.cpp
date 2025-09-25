#include <vm/VirtualMachine.h>
#include <vm/NativeRegistry.h>
#include <vm/ScriptContext.h>

namespace NVirtualMachine
{
    bool gRegisteredFunctions;



    bool RegisterNatives()
    {
        if (!gRegisteredFunctions)
        {
            gRegisteredFunctions = true;
        }

        return true;
    }

    void UnregisterNatives()
    {

    }

    bool Initialise() // 18
    {
        gScriptObjectManager = new CScriptObjectManager();
        return RegisterNatives();
    }

    void Finalise() // 30
    {
        UnregisterNatives();
        CleanupWorkingAreas();
        delete gScriptObjectManager;
        gScriptObjectManager = NULL;
    }

    CScriptObjectManager* GetScriptObjectManager()
    {
        return gScriptObjectManager;
    }
}