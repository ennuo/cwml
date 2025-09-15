#pragma once

struct EnumInfo
{
    int Value;
    const char* Name;
};

namespace NVirtualMachine
{
    typedef void (*NativeFunctionWrapper)(CScriptContext*, void*, u8*);

    #ifndef LBP1
        struct SNativeFunctionInfo
        {
            NativeFunctionWrapper Function;
            const char* Name;
            bool IsStatic;
        };

        struct SNativeEnum
        {
            const char* Name;
            EnumInfo* EnumInfoStart;
            EnumInfo* EnumInfoEnd;
        };

        struct SNativeClassExports
        {
            const char* Name;
            SNativeFunctionInfo* FunctionStart;
            SNativeFunctionInfo* FunctionEnd;
            SNativeEnum* EnumStart;
            SNativeEnum* EnumEnd;
        };

        extern SNativeClassExports* gClassExportsBegin;
        extern SNativeClassExports* gClassExportsEnd;
    #endif

    void RegisterNativeFunction(char* class_name, CSignature& signature, bool is_static, NativeFunctionWrapper function);
    void RegisterNativeFunction(char* class_name, char* function_signature, bool is_static, NativeFunctionWrapper function);
}