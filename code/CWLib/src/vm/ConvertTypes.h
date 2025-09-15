#pragma once

#include <vm/ScriptContext.h>
#include <vm/ScriptObject.h>

template <typename Type>
struct SConvertScriptTypes
{
    typedef Type NativeType;
    typedef Type VMType;

    static void VMToNative(NativeType& out, CScriptContext* context, VMType& in)
    {
        out = in;
    }

    static void NativeToVM(VMType& out, CScriptContext* context, NativeType& in)
    {
        out = in;
    }
};

