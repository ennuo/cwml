#include <vm/VMTypes.h>

struct SMachineTypeInfo
{
    const char* Name;
    u32 Size;
};

static SMachineTypeInfo gMachineTypeInfo[] =
{
    { "void", 0 },
    { "bool", 1 },
    { "char", 2 },
    { "s32", 4 },
    { "f32", 4 },
    { "v4", 16 },
    { "m44", 64 },
    { "", 0 },
    { "rawptr", 4 },
    { "refptr", 0 },
    { "safeptr", 4 },
    { "object", 4 },
#ifndef LBP1
    { "s64", 8 },
    { "f64", 8 }
#endif
};

namespace NVirtualMachine
{
    u32 GetTypeSize(EMachineType machine_type) 
    {
        if (machine_type < NUM_MACHINE_TYPES)
            return gMachineTypeInfo[machine_type].Size;
        return 0;
    }

    const char* GetTypeName(EMachineType machine_type)
    {
        if (machine_type < NUM_MACHINE_TYPES)
            return gMachineTypeInfo[machine_type].Name;
        return "";
    }
}