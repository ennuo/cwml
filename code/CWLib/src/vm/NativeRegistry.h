#pragma once

#include <MMString.h>
#include <vm/VMTypes.h>
#include <vector.h>

struct EnumInfo
{
    int Value;
    const char* Name;
};

namespace NVirtualMachine
{
    bool RegisterNatives();
    
    class CScriptContext;
    typedef void (*NativeFunctionWrapper)(CScriptContext*, void*, u8*);

    struct SNativeFunctionInfo // 14
    {
#ifdef LBP1
        SNativeFunctionInfo(const CSignature& signature, bool is_static, NativeFunctionWrapper function) :
        Signature(signature), IsStatic(is_static), Function(function)
        {

        }

        CSignature Signature;
        NativeFunctionWrapper Function;
#else
        NativeFunctionWrapper Function;
        const char* Name;
#endif
        bool IsStatic;
    };


    struct SNativeEnum // 36
    {
#ifdef LBP1
        const char* EnumName;
#else
        MMString<char> EnumName;
#endif
        EnumInfo* EnumInfoStart;
        EnumInfo* EnumInfoEnd;
    };


    struct SNativeClassExports { // 44
        typedef CVector<SNativeFunctionInfo> NativeFunctionVec;
        typedef CVector<SNativeEnum> NativeEnumVec;

        bool AddFunction(const CSignature&, bool, NativeFunctionWrapper);
        bool LookupFunction(const CSignature&, bool*, NativeFunctionWrapper*) const;
        bool AddEnum(const char*, EnumInfo*, const EnumInfo*);
        const SNativeEnum* LookupEnum(const char*) const;
        



        NativeFunctionVec NativeFunctions;
        NativeEnumVec NativeEnums;
    };

    #ifndef LBP1
        // struct SNativeClassExports
        // {
        //     const char* Name;
        //     SNativeFunctionInfo* FunctionStart;
        //     SNativeFunctionInfo* FunctionEnd;
        //     SNativeEnum* EnumStart;
        //     SNativeEnum* EnumEnd;
        // };

        // extern SNativeClassExports* gClassExportsBegin;
        // extern SNativeClassExports* gClassExportsEnd;
    #endif

    void RegisterNativeFunction(const char* class_name, const CSignature& signature, bool is_static, NativeFunctionWrapper function);
    // void RegisterNativeFunction(const char* class_name, const char* function_signature, bool is_static, NativeFunctionWrapper function);

    bool LookupNativeCall(const char* class_name, const CSignature& signature, bool* is_static, NativeFunctionWrapper* function);
}