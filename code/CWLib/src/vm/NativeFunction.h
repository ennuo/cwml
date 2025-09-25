#pragma once

#include <vm/ConvertTypes.h>

namespace NVirtualMachine
{
    class CScriptContext;

    template <int offset, typename T>
    struct TArgumentAlignment // 14
    {
        static const int value = (sizeof(T) + offset) - 1 & - sizeof(T);
    };

    template <typename T>
    struct ConvertToNative // 28
    {
        T NativeValue;
        
        template <int offset>
        ConvertToNative(CScriptContext* context, const u8* arguments, TArgumentAlignment<offset, T> alignment)
        {
            SConvertScriptTypes<T>::VMToNative(NativeValue, context, 
                *(typename SConvertScriptTypes<T>::VMType*)(arguments + alignment.value)
            );
        }
        
        operator const T&() const { return NativeValue; }
    };
}