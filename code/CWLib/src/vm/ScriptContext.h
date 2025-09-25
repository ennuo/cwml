#pragma once

#include <vector.h>
#include <thing.h>
#include <ParasiticVector.h>

#include <vm/VMTypes.h>
#include <vm/VMInstruction.h>
#include <vm/ScriptVariant.h>
#include <vm/ScriptArguments.h>
#include <vm/ScriptFunction.h>
#include <vm/ScriptInstance.h>

#include <ResourceScript.h>

namespace NVirtualMachine
{
    class CScriptObjectInstance;
    
    typedef uint32_t ScriptThingUID;

    const u32 WORKINGAREA_POOL_SIZE = 4;
    const u32 STACK_RESERVE_SIZE = 0x100;
    const u16 NO_RETURN_IDX = 0xFFFFu;
    const u32 STACK_SIZE = 10000;

    struct SWorkingArea
    {
        u8 Data[STACK_SIZE];
    };

    struct ExecutionState
    {
        ExecutionState(SWorkingArea*);
        
        SWorkingArea* WorkingArea;
        const RScript* Script;
        const CFunctionDefinitionRow* Function;
        CParasiticVector<const NVirtualMachine::Instruction> Bytecode;
        u8* Storage;
        u8* NewArgs;
        u32 PC;
        u32 NextPC;
        bool Finished;
        bool CanYield;
    };

    struct SActivationRecord {

        inline SActivationRecord() : Script(NULL), FunctionIdx(~0ul), PC(), BasePointer(),
        RVType(VMT_VOID), RVAddress(~0ul), CachedSelfThing(), CachedSelf()
        {

        }

        inline SActivationRecord(const RScript* script, u32 function_idx, u32 pc, u32 base_pointer, EMachineType rv_type, u32 rv_address) :
        Script(script), FunctionIdx(function_idx), PC(pc), BasePointer(base_pointer), RVType(rv_type), RVAddress(rv_address), CachedSelfThing(),
        CachedSelf()
        {

        }

        inline u32 GetBasePointer() const
        {
            return BasePointer;
        }

        inline u32 GetArg0Pointer() const
        {
            return BasePointer;
        }

        inline const CFunctionDefinitionRow* GetFunction() const
        {
            return Script->GetFunction(FunctionIdx);
        }

        const RScript* Script;
        u32 FunctionIdx;
        u32 PC;
        u32 BasePointer;
        EMachineType RVType;
        u32 RVAddress;
        CThingPtr CachedSelfThing;
        bool CachedSelf;
    };

    enum EScriptExecutionState
    {
        SES_OK,
        SES_FINISHED_OK,
        SES_FINISHED_ERRORED
    };

    class CScriptContext {
    public:
        inline CScriptContext() : WorldThing(), SavedStack(), ActivationRecords(), CachedSelfThingUID(),
        Result(), State(), YieldASAP()
        {
            SavedStack.reserve(STACK_RESERVE_SIZE);
            ActivationRecords.reserve(8);
        }

        ~CScriptContext();
    public:
        void Reset();
        bool PrepareStaticFunction(CThing*, const CScriptFunctionBinding&, const CScriptArguments&);
        bool PrepareNonStaticFunction(CThing*, const CScriptFunctionBinding&, const CThingPtr&, const CScriptArguments&);
        bool PrepareNonStaticFunction(CThing*, const CScriptFunctionBinding&, const CScriptObjectInstance*, const CScriptArguments&);
        bool PrepareNonStaticFunction();
        bool RunToYield();
        bool RunToCompletion();
        void YieldScript();
        bool Finished() const;
        bool FinishedSuccessfully() const;
        CScriptVariant GetResult() const;
        CThing* LookupThing(ScriptThingUID) const;
        void GatherObjectReferences(CRawVector<unsigned int>&) const;
        bool StackFrameByteswap(CRawVector<unsigned char>&) const;
        bool StackFrameNToH(CRawVector<unsigned char>&) const;
        bool StackFrameHToN(CRawVector<unsigned char>&) const;
        u32 GetNumActivationRecords() const;
        const SActivationRecord* GetActivationRecord(u32) const;
        void UpgradeScripts(PWorld*);
        CThing* GetWorldThing();
        PWorld* GetWorld();
        void DumpScriptStack(ByteArray&);
        const RScript* GetEntryPoint() const;
        bool PrepareFunction(CThing*, const CScriptFunctionBinding&, const CScriptArguments&, bool, u32);
        bool PushFunctionX(const CScriptFunctionBinding&, EMachineType);
        bool PushFunction(ExecutionState& state, const CScriptFunctionBinding& binding, u32 current_pc, EMachineType rv_type, u16 return_idx);
        bool PopFunction();
        void GetState(ExecutionState&) const;
        void SetState(const ExecutionState&);
        bool Run(u32);
        CScriptInstance* GetInstanceFromThing(ScriptThingUID);
        CScriptInstance* GetInstanceFromObject(ScriptObjectUID);
        void GetMember(const RScript*, u32, const CScriptInstance&, EMachineType, void*);
        void SetMember(const RScript*, u32, CScriptInstance&, EMachineType, const void*);
        void GetNativeMemberThing(EMachineType, ScriptThingUID, const RScript*, u32, void*);
        void GetMemberThing(EMachineType, ScriptThingUID, const RScript*, u32, void*);
        void SetMemberThing(EMachineType, ScriptThingUID, const RScript*, u32, const void*);
        void GetObjectMember(EMachineType, ScriptObjectUID, const RScript*, u32, void*);
        void SetObjectMember(EMachineType machine_type, ScriptObjectUID object_uid, const RScript* script, u32 field_ref_idx, const void* data);
        void InvokeFunction(ExecutionState& state, EMachineType call_type, u32 dst_idx, u32 call_idx);

        void HandleScriptException(const char* msg);
        void Write(EMachineType machine_type, const void* data) const;
        void FormatAsString(MMOTextStreamA& stream, EMachineType machine_type, const void* data) const;
    public:
        CThingPtr WorldThing;
        CRawVector<unsigned char> SavedStack;
        CVector<SActivationRecord> ActivationRecords;
        ScriptThingUID CachedSelfThingUID;
        CScriptVariant Result;
        EScriptExecutionState State;
        bool YieldASAP;
    };

    SWorkingArea* AcquireWorkingArea();
    void ReleaseWorkingArea(SWorkingArea* area);
    void CleanupWorkingAreas();
}