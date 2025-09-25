#include <vm/ScriptFunction.h>
#include <vm/ScriptContext.h>
#include <ResourceScript.h>
#include <DebugLog.h>

namespace NVirtualMachine
{
    bool CScriptFunctionBinding::InvokeSync(PWorld* pworld, CScriptObjectInstance* self, const CScriptArguments& arguments, CScriptVariant* return_value) // 107
    {
        return TryInvokeSync(pworld, self, arguments, return_value);
    }

    bool CScriptFunctionBinding::TryInvokeSync(PWorld* pworld, CScriptObjectInstance* self, const CScriptArguments& arguments, CScriptVariant* return_value) // 243
    {
        if (!IsValid()) return false;

        // TODO: do proper shit later i just dont particularly care about the pworld requirement
        // CScriptContext* context = pworld->AcquireScriptContext();
        CScriptContext* context = new CScriptContext();
        if (!context->PrepareNonStaticFunction(NULL /* pworld->GetWorldThing() */, *this, self, arguments))
            return false;

        context->RunToCompletion();

        

            

        // pworld->ReleaseScriptContext(context);

        return false;
    }
}