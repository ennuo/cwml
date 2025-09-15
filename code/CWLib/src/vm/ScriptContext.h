#pragma once

#include <vector.h>
#include <thing.h>

#include <vm/VMTypes.h>
#include <vm/VMInstruction.h>

namespace NVirtualMachine
{
    typedef uint32_t ScriptThingUID;

    struct SWorkingArea
    {
        u8 Data[10000];
    };

    struct ExecutionState
    {
        SWorkingArea* WorkingArea;
        void* Script;
        void* Function;
        Instruction* Bytecode;
        u32 BytecodeSize;
        u32 BytecodeMaxSize;
        u8* Storage;
        u8* NewArgs;
        u32 PC;
        u32 NextPC;
        bool Finished;
        bool CanYield;
    };

    struct SActivationRecord
    {
        void* Script;
        u32 FunctionIdx;
        u32 PC;
        u32 BasePointer;
        EMachineType RVType;
        u32 RVAddress;
        CThingPtr CachedSelfThing;
        bool CachedSelf;
    };

    class CScriptContext {
    public:
        CThingPtr WorldThing;
        CRawVector<unsigned char> SavedStack;
        CVector<SActivationRecord> ActivationRecords;
        ScriptThingUID CachedSelfThingUID;
    };
}