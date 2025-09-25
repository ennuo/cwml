#include <vm/ScriptContext.h>
#include <vm/ScriptObject.h>
#include <vm/VirtualMachine.h>
#include <ResourceScript.h>
#include <thread.h>

namespace NVirtualMachine
{
    CRawVector<SWorkingArea*> gWorkingAreaPool;

    SWorkingArea* AcquireWorkingArea() // 58
    {
        SWorkingArea* area;
        if (gWorkingAreaPool.empty())
        {
            area = new SWorkingArea();
            memset(area->Data, 0, STACK_SIZE);
        }
        else
        {
            area = gWorkingAreaPool.pop_back();
        }

        return area;
    }

    void ReleaseWorkingArea(SWorkingArea* area) // 77
    {
        if (gWorkingAreaPool.size() < WORKINGAREA_POOL_SIZE)
        {
            gWorkingAreaPool.push_back(area);
        }
        else
        {
            delete area;
        }
    }

    void CleanupWorkingAreas() // 90
    {
        for (u32 i = 0; i < gWorkingAreaPool.size(); ++i)
            delete gWorkingAreaPool[i];
        gWorkingAreaPool.clear();
    }

    ExecutionState::ExecutionState(SWorkingArea* area) :
    WorkingArea(area), Script(), Function(), Bytecode(),
    Storage(), NewArgs(), PC(), NextPC(), Finished(), CanYield()
    {

    }

    CScriptContext::~CScriptContext() // 146
    {

    }

    void CScriptContext::GetState(ExecutionState& state) const
    {
        const SActivationRecord& activation_record = ActivationRecords.back();
        state.Script = activation_record.Script;
        state.Function = activation_record.GetFunction();
        state.Bytecode = state.Script->GetFunctionBytecode(state.Function);
        state.Storage = state.WorkingArea->Data + activation_record.GetBasePointer();
        state.NewArgs = state.WorkingArea->Data + activation_record.GetBasePointer() + state.Function->StackSize;
        state.PC = activation_record.PC;
    }

    void CScriptContext::SetState(const ExecutionState& state)
    {
        ActivationRecords.back().PC = state.PC;
    }

    void CScriptContext::Reset() // 216
    {
        WorldThing = NULL;
        SavedStack.resize(0);
        ActivationRecords.resize(0);
        CachedSelfThingUID = 0;
        Result.Clear();
        State = SES_OK;
        YieldASAP = false;
    }

    bool CScriptContext::PrepareNonStaticFunction(CThing* world_thing, const CScriptFunctionBinding& binding, const CScriptObjectInstance* object, const CScriptArguments& args) // 253
    {
        ScriptObjectUID object_uid = object->GetUID();
        return PrepareFunction(world_thing, binding, args, true, object_uid.UID);
    }

    bool CScriptContext::PrepareFunction(CThing* world_thing, const CScriptFunctionBinding& binding, const CScriptArguments& args, bool push_arg0, u32 arg0) // 267
    {
        Reset();

        WorldThing = world_thing;

        const CP<RScript>& entry_script = binding.GetScript();
        entry_script->Fixup();
        bool expect_static = binding.GetFunction()->Modifiers.IsSet(MT_STATIC);
        
        if (push_arg0 && expect_static) return false;
        if (!PushFunctionX(binding, VMT_VOID)) return false;

        const SActivationRecord& activation_record = ActivationRecords.back();

        const RScript* script = activation_record.Script;
        const CFunctionDefinitionRow* function = activation_record.GetFunction();

        if (args.GetCount() != function->GetNumArguments())
            return false;
        
        if (push_arg0)
            *(u32*)(SavedStack.begin() + activation_record.GetArg0Pointer()) = arg0;
        
        u8* storage = SavedStack.begin() + activation_record.GetBasePointer();
        for (u32 i = 0; i < function->GetNumArguments(); ++i)
        {
            const STypeOffset& type_offset = script->GetFunctionArgument(function, i);
            const CTypeReferenceRow* type_ref = script->GetType(type_offset.TypeReferenceIdx);
            const CScriptVariant& argument = args.GetArgument(i);
            void* addr = (void*)(storage + type_offset.Offset);

            switch (type_ref->GetMachineType())
            {
                // TODO: implement
            }
        }

        return true;
    }

    bool CScriptContext::PushFunctionX(const CScriptFunctionBinding& binding, EMachineType rv_type) // 359
    {
        const CP<RScript>& script = binding.GetScript();
        if (!script) return false;

        const CFunctionDefinitionRow* function = binding.GetFunction();

        u32 current_sp = 0;
        u32 rv_address = ~0ul;
        u32 first_local = 0;
        u32 base_pointer = 0;
        u32 stack_size = 0;
        u32 total_stack_size = 0;

        if (function->GetNumArguments() != 0)
        {
            STypeOffset arg = script->GetFunctionArgument(function, function->GetNumArguments() - 1);
            const CTypeReferenceRow* type_ref = script->GetType(arg.TypeReferenceIdx);
            first_local = arg.Offset + GetTypeSize(type_ref->GetMachineType());
        }
        else
        {
            first_local = function->Modifiers.IsSet(MT_STATIC) ? 0 : GetTypeSize(VMT_SAFE_PTR);
        }

        stack_size = function->StackSize;
        total_stack_size = base_pointer + stack_size;

        SavedStack.resize(total_stack_size);

        if (STACK_SIZE < total_stack_size)
        {
            HandleScriptException("StackOverflow");
            return false;
        }

        if (stack_size > first_local)
            memset(SavedStack.begin() + base_pointer + first_local, 0, stack_size - first_local);

        SActivationRecord activation_record(script, binding.GetFunctionIndex(), 0, base_pointer, rv_type, rv_address);
        ActivationRecords.push_back(activation_record);

        return true;
    }

    bool CScriptContext::PushFunction(ExecutionState& state, const CScriptFunctionBinding& binding, u32 current_pc, EMachineType rv_type, u16 return_idx) // 427
    {
        const CP<RScript>& script = binding.GetScript();
        if (!script) return false;
        const CFunctionDefinitionRow* function = binding.GetFunction();

        u32 current_sp = 0;
        u32 rv_address = ~0ul;
        u32 first_local = 0;
        u32 base_pointer = 0;
        u32 stack_size = 0;
        u32 total_stack_size = 0;

        if (ActivationRecords.size() != 0)
        {
            SActivationRecord& activation_record = ActivationRecords.back();
            activation_record.PC = current_pc;

            base_pointer = activation_record.BasePointer + activation_record.GetFunction()->StackSize;
            if (rv_type != VMT_VOID)
                rv_address = activation_record.BasePointer + return_idx;
        }

        if (function->GetNumArguments() != 0)
        {
            STypeOffset arg = script->GetFunctionArgument(function, function->GetNumArguments() - 1);
            const CTypeReferenceRow* type_ref = script->GetType(arg.TypeReferenceIdx);
            first_local = arg.Offset + GetTypeSize(type_ref->GetMachineType());
        }
        else
        {
            first_local = function->Modifiers.IsSet(MT_STATIC) ? 0 : GetTypeSize(VMT_SAFE_PTR);
        }

        stack_size = function->StackSize;
        total_stack_size = base_pointer + stack_size;

        if (STACK_SIZE < total_stack_size)
        {
            HandleScriptException("StackOverflow");
            return false;
        }

        if (stack_size > first_local)
            memset(SavedStack.begin() + base_pointer + first_local, 0, stack_size - first_local);

        SActivationRecord activation_record(script, binding.GetFunctionIndex(), 0, base_pointer, rv_type, rv_address);
        ActivationRecords.push_back(activation_record);

        return true;
    }

    bool CScriptContext::PopFunction()
    {
        ActivationRecords.pop_back();
        return ActivationRecords.size() == 0;
    }

    CScriptInstance* CScriptContext::GetInstanceFromObject(ScriptObjectUID object_uid) // 522
    {
        CScriptObject* object = GetScriptObjectManager()->LookupObject(object_uid);
        if (object == NULL)
        {
            HandleScriptException(object_uid == 0 ? "NullObject" : "InvalidObject");
            return NULL;
        }

        if (!object->IsInstance())
        {
            HandleScriptException("InvalidObjectType");
            return NULL;
        }

        return &((CScriptObjectInstance*)object)->GetInstance();
    }

    void CScriptContext::SetMember(const RScript* script, u32 field_ref_idx, CScriptInstance& instance, EMachineType machine_type, const void* data)
    {
        u32 offset = script->GetFieldReference(field_ref_idx)->InstanceOffset;

        if (offset == INVALID_FIELD_OFFSET)
        {
            HandleScriptException("FieldUnbound");
            return;
        }

        if (!instance.SetMember(machine_type, offset, data))
            HandleScriptException("InvalidField");
    }

    void CScriptContext::SetObjectMember(EMachineType machine_type, ScriptObjectUID object_uid, const RScript* script, u32 field_ref_idx, const void* data) // 691
    {
        CScriptInstance* instance = GetInstanceFromObject(object_uid);
        if (instance != NULL)
            SetMember(script, field_ref_idx, *instance, machine_type, data);
    }


    bool CScriptContext::RunToCompletion()
    {
        return Run(~0ul);
    }

    void CScriptContext::InvokeFunction(ExecutionState& state, EMachineType call_type, u32 dst_idx, u32 call_idx)
    {
        const CFunctionReferenceRow* function_ref = state.Script->GetFunctionReference(call_idx);
        
        bool call_ok;
        const char* function_name = state.Script->LookupStringA(function_ref->NameStringIdx);
        
        if (function_ref->BoundFunctionNative == NULL)
        {
            if (!function_ref->BoundFunction.IsValid() || !PushFunction(state, function_ref->BoundFunction, state.NextPC, call_type, dst_idx))
            {
                MMString<char> s("FunctionCallNotBound: ");
                s += function_name;
                s += ". Do you need a new build of the game?";
                HandleScriptException(s.c_str());
                return;
            }

            GetState(state);
            state.NextPC = state.PC;
        }
        else
        {
            function_ref->BoundFunctionNative(this, state.Storage + dst_idx, state.NewArgs);
            if (state.CanYield && YieldASAP)
                state.Finished = true;
        }
    }

    bool CScriptContext::Run(u32 num_cycles) // 1300
    {
        if (State != SES_OK || !AmInMainThread()) return false;

        SWorkingArea* working_area = AcquireWorkingArea();
        memcpy(working_area->Data, SavedStack.begin(), SavedStack.size());
        ExecutionState state(working_area);
        GetState(state);
        YieldASAP = false;

        bool run_to_yield_or_completion = true;
        u32 instructions_executed = 0;
        
        while (run_to_yield_or_completion)
        {
            if (1 > num_cycles + 2 && num_cycles <= instructions_executed)
                break;

            state.NextPC = state.PC + 1;
            NVirtualMachine::Instruction inst = state.Bytecode[state.PC];
    
#define IT_GET_STORAGE_POINTER(type, index) (type*)(state.Storage + index)
#define IT_GET_STORAGE_VALUE(type, index) *(type*)(state.Storage + index)
#define IT_SET_STORAGE_VALUE(type, dest, value) *(IT_GET_STORAGE_POINTER(type, dest)) = value
#define IT_MOVE_STORAGE_VALUE(type, source, dest) *(IT_GET_STORAGE_POINTER(type, dest)) = IT_GET_STORAGE_VALUE(type, source)
#define IT_MOVE_UNARY(type) IT_MOVE_STORAGE_VALUE(type, inst.Unary.SrcIdx, inst.Unary.DstIdx)
#define IT_MIN(type) IT_SET_STORAGE_VALUE(type, inst.Binary.DstIdx, MIN_MACRO(IT_GET_STORAGE_VALUE(type, inst.Binary.SrcAIdx), IT_GET_STORAGE_VALUE(type, inst.Binary.SrcBIdx)))
#define IT_MAX(type) IT_SET_STORAGE_VALUE(type, inst.Binary.DstIdx, MAX_MACRO(IT_GET_STORAGE_VALUE(type, inst.Binary.SrcAIdx), IT_GET_STORAGE_VALUE(type, inst.Binary.SrcBIdx)))
#define IT_SIMPLE_CAST(source_type, dest_type, dest, source) IT_SET_STORAGE_VALUE(dest_type, dest, (dest_type)IT_GET_STORAGE_VALUE(source_type, source))


#define IT_UNARY_TYPED(input_type, output_type, operation) IT_SET_STORAGE_VALUE(output_type, inst.Unary.DstIdx, operation(IT_GET_STORAGE_VALUE(input_type, inst.Unary.SrcIdx)))
#define IT_UNARY(type, operation) IT_UNARY_TYPED(type, type, operation)

#define IT_BINARY_TYPED(input_type, output_type, operation) IT_SET_STORAGE_VALUE(output_type, inst.Binary.DstIdx, IT_GET_STORAGE_VALUE(input_type, inst.Binary.SrcAIdx) operation IT_GET_STORAGE_VALUE(input_type, inst.Binary.SrcBIdx))
#define IT_BINARY(type, operation) IT_BINARY_TYPED(type, type, operation)
#define IT_COMPARISON(type, operation) IT_BINARY_TYPED(type, bool, operation);

            switch (inst.Unary.Op)
            {
                case IT_NOP: break;
                case IT_LC_B:
                {
                    IT_SET_STORAGE_VALUE(bool, inst.LoadConstBool.DstIdx, inst.LoadConstBool.Bool);
                    break;
                }
                case IT_LC_C:
                {
                    IT_SET_STORAGE_VALUE(wchar_t, inst.LoadConstChar.DstIdx, inst.LoadConstChar.Char);
                    break;
                }
                case IT_LC_I:
                {
                    IT_SET_STORAGE_VALUE(s32, inst.LoadConstInt.DstIdx, inst.LoadConstInt.Int);
                    break;
                }
                case IT_LC_F:
                {
                    IT_SET_STORAGE_VALUE(float, inst.LoadConstFloat.DstIdx, inst.LoadConstFloat.Float);
                    break;
                }

                // case IT_LC_SW: break;
                // case IT_LC_NULL_SP: break;
                case IT_MOV_B: IT_MOVE_UNARY(bool); break;
                case IT_LOG_NEG_B: IT_UNARY(bool, !); break;
                case IT_MOV_C: IT_MOVE_UNARY(wchar_t); break;
                case IT_MOV_I: IT_MOVE_UNARY(s32); break;
                case IT_INC_I: IT_UNARY(s32, ++); break;
                case IT_DEC_I: IT_UNARY(s32, --);
                case IT_NEG_I: IT_UNARY(s32, -); break;
                case IT_BIT_NEG_I: IT_UNARY(s32, ~); break;
                case IT_LOG_NEG_I: IT_UNARY(s32, !); break;
                case IT_ABS_I: IT_UNARY(s32, abs); break;
                case IT_MOV_F: IT_MOVE_UNARY(float); break;
                case IT_NEG_F: IT_UNARY(float, -); break;
                case IT_ABS_F: IT_UNARY(float, abs); break;
                // case IT_SQRT_F: break;
                // case IT_SIN_F: break;
                // case IT_COS_F: break;
                // case IT_TAN_F: break;
                case IT_MOV_V4: IT_MOVE_UNARY(v4); break;
                // case IT_NEG_V4: break;
                // case IT_MOV_S_DEPRECATED: break;
                // case IT_MOV_RP: break;
                // case IT_MOV_CP: break;
                // case IT_MOV_O: break;
                case IT_EQ_B: IT_BINARY(bool, ==); break;
                case IT_NE_B: IT_BINARY(bool, !=); break;
                // case IT_RESERVED0_C: break;
                // case IT_RESERVED1_C: break;
                case IT_LT_C: IT_BINARY(wchar_t, <); break;
                case IT_LTE_C: IT_BINARY(wchar_t, <=); break;
                case IT_GT_C: IT_BINARY(wchar_t, >); break;
                case IT_GTE_C: IT_BINARY(wchar_t, >=); break;
                case IT_EQ_C: IT_BINARY(wchar_t, ==); break;
                case IT_NE_C: IT_BINARY(wchar_t, !=); break;
                case IT_ADD_I: IT_BINARY(s32, +); break;
                case IT_SUB_I: IT_BINARY(s32, -); break;
                case IT_MUL_I: IT_BINARY(s32, *); break;
                case IT_DIV_I: IT_BINARY(s32, /); break;
                case IT_MOD_I: IT_BINARY(s32, %); break;
                case IT_MIN_I: IT_MIN(s32); break;
                case IT_MAX_I: IT_MAX(s32); break;
                case IT_SLA_I: IT_BINARY(s32, >>); break;
                case IT_SRA_I: IT_BINARY(s32, <<); break;
                case IT_BIT_OR_I: IT_BINARY(s32, |); break;
                case IT_BIT_AND_I: IT_BINARY(s32, &); break;
                case IT_BIT_XOR_I: IT_BINARY(s32, ^); break;
                case IT_LT_I: IT_COMPARISON(s32, <); break;
                case IT_LTE_I: IT_COMPARISON(s32, <=); break;
                case IT_GT_I: IT_COMPARISON(s32, >); break;
                case IT_GTE_I: IT_COMPARISON(s32, >=); break;
                case IT_EQ_I: IT_COMPARISON(s32, ==); break;
                case IT_NE_I: IT_COMPARISON(s32, !=); break;
                case IT_ADD_F: IT_BINARY(float, +); break;
                case IT_SUB_F: IT_BINARY(float, -); break;
                case IT_MUL_F: IT_BINARY(float, *); break;
                case IT_DIV_F: IT_BINARY(float, /); break;
                case IT_MIN_F: IT_MIN(float); break;
                case IT_MAX_F: IT_MAX(float); break;
                case IT_LT_F: IT_COMPARISON(float, <); break;
                case IT_LTE_F: IT_COMPARISON(float, <=); break;
                case IT_GT_F: IT_COMPARISON(float, >); break;
                case IT_GTE_F: IT_COMPARISON(float, >=); break;
                case IT_EQ_F: IT_COMPARISON(float, ==); break;
                case IT_NE_F: IT_COMPARISON(float, !=); break;
                // blah blah
                
                case IT_WRITE:
                {
                    Write(
                        (EMachineType)inst.Write.Type,
                        (const void*)(state.Storage + inst.Write.SrcIdx)
                    );

                    break;
                }

                case IT_ARG:
                {
                    memcpy(
                        state.NewArgs + inst.Arg.ArgIdx,
                        state.Storage + inst.Arg.SrcIdx,
                        GetTypeSize((EMachineType)inst.Arg.Type)
                    );

                    break;
                }

                case IT_CALL:
                {
                    EMachineType call_type = (EMachineType)inst.Call.Type;
                    u32 dst_idx = inst.Call.DstIdx;
                    u32 call_idx = inst.Call.CallIdx;
                    InvokeFunction(state, call_type, dst_idx, call_idx);
                    break;
                }

                case IT_RET:
                {
                    const SActivationRecord& activation_record = ActivationRecords.back();
                    if (activation_record.RVAddress != ~0ul)
                    {
                        memcpy(
                            state.WorkingArea->Data + activation_record.RVAddress,
                            state.Storage + inst.Return.SrcIdx,
                            GetTypeSize(activation_record.RVType)
                        );
                    }

                    if (PopFunction())
                    {
                        run_to_yield_or_completion = false;
                        State = SES_FINISHED_OK;
                    }
                    else
                    {
                        GetState(state);
                        state.NextPC = state.PC;
                    }

                    break;
                }

                case IT_B:
                {
                    state.NextPC = state.PC + inst.Branch.BranchOffset;
                    break;
                }

                case IT_BEZ:
                {
                    if (!IT_GET_STORAGE_VALUE(bool, inst.Branch.SrcIdx))
                        state.NextPC = state.PC + inst.Branch.BranchOffset;
                    break;
                }

                case IT_BNEZ:
                {
                    if (IT_GET_STORAGE_VALUE(bool, inst.Branch.SrcIdx))
                        state.NextPC = state.PC + inst.Branch.BranchOffset;
                    break;
                }

                case IT_SET_OBJ_MEMBER:
                {
                    SetObjectMember(
                        (EMachineType)inst.SetMember.Type,
                        *(ScriptObjectUID*)(state.Storage + inst.SetMember.BaseIdx),
                        state.Script,
                        inst.SetMember.FieldRef,
                        (void*)(state.Storage + inst.SetMember.SrcIdx)
                    );
                    
                    break;
                }

                case IT_LC_SA:
                {
                    const char* string = state.Script->LookupStringA(inst.LoadConstString.StringIdx);
                    ScriptObjectUID uid = GetScriptObjectManager()->RegisterStringA(string);
                    *(ScriptObjectUID*)(state.Storage + inst.LoadConstString.DstIdx) = uid;
                    
                    break;
                }

                default:
                {
                    char buf[512];
                    FormatString(buf, "UnsupportedInstruction (%08x) at:\n", inst.Unary.Op);
                    HandleScriptException(buf);
                    break;
                }
            }

            if (State != SES_OK) break;

            state.PC = state.NextPC;
            instructions_executed++;
        }

        if (State == SES_OK)
            SetState(state);

        if (ActivationRecords.empty()) SavedStack.resize(0);
        else
        {
            SActivationRecord& activation_record = ActivationRecords.back();
            SavedStack.resize(activation_record.GetFunction()->StackSize);
            memcpy(SavedStack.begin(), working_area, SavedStack.size());
        }

        ReleaseWorkingArea(working_area);
        return State != SES_FINISHED_ERRORED;
    }

    void CScriptContext::HandleScriptException(const char* exception) // 2208
    {
        MMLogCh(DC_SCRIPT, "Script exception '%s' at:\n", exception);
        ByteArray b;
        DumpScriptStack(b);
        MMLogCh(DC_SCRIPT, "%s", b.begin());
        State = SES_FINISHED_ERRORED;
    }

    void CScriptContext::DumpScriptStack(ByteArray& stack)
    {
        char buf[512];

        for (s32 i = ActivationRecords.size() - 1; i >= 0; --i)
        {
            SActivationRecord& record = ActivationRecords[i];
            const CFunctionDefinitionRow* function = record.GetFunction();
            const char* name = record.Script->LookupStringA(function->NameStringIdx);

            if (record.PC < function->GetBytecodeSize())
            {
                FormatString(buf, "%s.%s(%d)", record.Script->GetClassName(), name, record.Script->GetFunctionLineNo(function, record.PC));
                if (ActivationRecords.size() - 1 == i)
                    StringAppend(buf, " (this line number may be incorrect in optimised builds)");
                StringAppend(buf, "\r\n");
            }
            else
            {
                FormatString(buf, "%s.%s+%d\r\n", record.Script->GetClassName(), name, record.PC);
            }

            stack.insert(stack.end(), buf, buf + StringLength(buf));
        }

        stack.push_back('\0');
    }

    void CScriptContext::Write(EMachineType machine_type, const void* data) const
    {
        MMOTextStreamString stream;
        FormatAsString(stream, machine_type, data);
        MMLogCh(DC_SCRIPT, "%s", stream.CStr());
    }

    void CScriptContext::FormatAsString(MMOTextStreamA& stream, EMachineType machine_type, const void* data) const
    {
        switch (machine_type)
        {
            case VMT_BOOL: stream << *(bool*)data; break;
            case VMT_CHAR: stream << *(wchar_t*)data; break;
            case VMT_S32: stream << *(int*)data; break;
            case VMT_F32: stream << *(float*)data; break;
            // case VMT_VECTOR4: break;
            // case VMT_M44: break;
            case VMT_OBJECT_REF:
            {
                CScriptObject* object = GetScriptObjectManager()->LookupObject(*(ScriptObjectUID*)data);
                if (object != NULL)
                    object->Stream(stream);
                break;
            }

            default: break;
        }
    }
}