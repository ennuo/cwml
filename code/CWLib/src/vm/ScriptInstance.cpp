#include <vm/ScriptObject.h>
#include <ResourceScript.h>
#include <PartPhysicsWorld.h>

namespace NVirtualMachine
{
    CSignature gInitSig(".init__");
    CSignature gCtorSig(".ctor__");

    CScriptInstance::CScriptInstance() : Script(), InstanceLayout(), MemberVariables() // 22
    {

    }

    CScriptInstance::CScriptInstance(const CP<RScript>& script) : // 31
    Script(script), InstanceLayout(), MemberVariables()
    {

    }

    CScriptInstance::~CScriptInstance() // 40
    {

    }

    void CScriptInstance::SetScript(const CP<RScript>& script) // 48
    {
        Script = script;
        InstanceLayout = NULL;
        MemberVariables.resize(0);
    }

    bool CScriptInstance::NeedsGCScan() const
    {
        return InstanceLayout ? InstanceLayout->NeedsGCScan() : true;
    }

    void CScriptInstance::InitialiseMemberData(CScriptObjectInstance* object, PWorld* pworld, bool default_construct) // 138
    {
        if (!Script) return;

        InstanceLayout = Script->GetInstanceLayout();

        u32 size = InstanceLayout->GetInstanceSize();

        MemberVariables.resize(size);
        if (size != 0)
            memset(MemberVariables.begin(), 0, size);
        
        CScriptFunctionBinding binding;
        CScriptArguments args;
        if (Script->LookupFunction(gInitSig, &binding))
            binding.TryInvokeSync(pworld, object, args, NULL);

        if (default_construct)
        {
            if (Script->LookupFunction(gCtorSig, &binding))
                binding.TryInvokeSync(pworld, object, args, NULL);
        }
    }

    bool CScriptInstance::ElementInRange(u32 offset, EMachineType machine_type) const
    {
        return offset < MemberVariables.size() && offset + GetTypeSize(machine_type) <= MemberVariables.size();
    }

    void MachineTypeAssign(EMachineType machine_type, void* dst, const void* src)
    {
        switch (machine_type)
        {
            case VMT_BOOL: *(bool*)dst = *(bool*)src; break;
            case VMT_CHAR: *(wchar_t*)dst = *(wchar_t*)src; break;
            case VMT_F32: *(float*)dst = *(float*)src; break;
            case VMT_VECTOR4: *(v4*)dst = *(v4*)src; break;
            case VMT_M44: *(m44*)dst = *(m44*)src; break;
            case VMT_S32:
            case VMT_RAW_PTR: 
            case VMT_REF_PTR:
            case VMT_SAFE_PTR:
            case VMT_OBJECT_REF:
                *(u32*)dst = *(u32*)src;
                break;
#ifndef LBP1
            case VMT_S64: *(u64*)dst = *(u64*)src; break;
            case VMT_F64: *(double*)dst = *(double*)src; break;
#endif
        }
    }

    bool CScriptInstance::SetMember(EMachineType machine_type, u32 offset, const void* data) // 409
    {
        if (!ElementInRange(offset, machine_type)) return false;
        MachineTypeAssign(machine_type, MemberVariables.begin() + offset, data);
        return true;
    }

    bool CScriptInstance::GetMember(EMachineType machine_type, void* data, u32 offset) const
    {
        if (!ElementInRange(offset, machine_type)) return false;
        MachineTypeAssign(machine_type, data, MemberVariables.begin() + offset);
        return true;
    }
}