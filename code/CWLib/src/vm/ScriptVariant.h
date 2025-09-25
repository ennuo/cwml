#pragma once

#include <vm/VMTypes.h>

class CScriptObject;

class CScriptVariant {
public:
    inline CScriptVariant() : MachineType(VMT_VOID), Bits(0) {}
    inline CScriptVariant(bool value) : MachineType(VMT_BOOL), Bool(value) {}
    inline CScriptVariant(s32 value) : S32(value), MachineType(VMT_S32) {}
    inline CScriptVariant(u32 value) : S32(value), MachineType(VMT_S32) {}
    inline CScriptVariant(float value) : F32(value), MachineType(VMT_F32) {}
public:
    inline void Clear()
    {
        memset(this, 0, sizeof(CScriptVariant));
    }
private:
    EMachineType MachineType;
    union
    {
        u32 Bits;
        bool Bool;
        wchar_t Char;
        s32 S32;
        float F32;
        const wchar_t* String;
        u32 ThingUID;
        u32 ScriptUID;
#ifndef LBP1
        s64 S64;
        double F64;
#endif
    };

    m44 M44;
    v4 V4;
};
