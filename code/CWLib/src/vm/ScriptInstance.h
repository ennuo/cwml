#pragma once

#include <refcount.h>
#include <vector.h>
#include <vm/InstanceLayout.h>
#include <vm/ScriptFunction.h>

#include <utility>


class RScript;
class CInstanceLayout;
class CThing;
class CScriptObjectInstance;
class PWorld;

namespace NVirtualMachine
{
    class CScriptInstance {
    public:
        CScriptInstance();
        CScriptInstance(const CP<RScript>&);
        ~CScriptInstance();
    public:
        inline const CP<RScript>& GetScript() const { return Script; }
        inline CRawVector<unsigned char>& GetMembers() { return MemberVariables; }
        inline const CRawVector<unsigned char>& GetMembers() const { return MemberVariables; }
    public:
        void SetScript(const CP<RScript>&);
        bool NeedsGCScan() const;
        bool IsMemberDataValid() const;
        bool InvalidateMemberVariables();
        void InitialiseMemberData(CThing*);
        void InitialiseMemberData(CScriptObjectInstance*, PWorld*, bool);
        bool UpgradeMemberData(CThing*);
        bool UpgradeMemberData(CThing*, CScriptObjectInstance*, PWorld*);
        void RemapThingUIDs(const CRawVector<std::pair<u32, u32> >&);
        void GatherObjectReferences(CRawVector<unsigned int>&);
        CScriptFunctionBinding Bind(const CScriptInstance*, const CSignature&);
        bool SetMember(EMachineType machine_type, u32 offset, const void* data);
        bool GetMember(EMachineType, void*, u32) const;
    public:
        inline const CP<CInstanceLayout>& GetInstanceLayout() const { return InstanceLayout; }
        inline bool IsDivergent() const;
    public:
        CScriptInstance(const CScriptInstance&);
        CScriptObjectInstance& operator=(const CScriptInstance&);
    public:
        bool ElementInRange(u32, EMachineType) const;
    public:
        CP<RScript> Script;
        CP<CInstanceLayout> InstanceLayout;
        CRawVector<unsigned char> MemberVariables;
    };
}