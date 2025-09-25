#pragma once

#include <vm/ScriptObject.h>
#include <vm/ScriptContext.h>

#include <ResourceScript.h>

namespace NVirtualMachine
{
    bool Initialise();
    void Finalise();

    CScriptObjectManager* GetScriptObjectManager();
}
