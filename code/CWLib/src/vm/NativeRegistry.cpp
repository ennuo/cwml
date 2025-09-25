#include <vm/NativeRegistry.h>
#include <vm/VirtualMachine.h>

#include <map>
#include <algorithm>

namespace NVirtualMachine
{
#ifdef WIN32
    typedef std::map<MMString<char>, SNativeClassExports> ClassNativeFunctionMap;
    ClassNativeFunctionMap gRegisteredNativeFunctionMap;

    struct SCompareNativeFunctionSigs
    {
        inline bool operator()(const SNativeFunctionInfo& lhs, const SNativeFunctionInfo& rhs)
        {
            return lhs.Signature.GetMangledNameHash() < rhs.Signature.GetMangledNameHash();
        }

        inline bool operator()(const SNativeFunctionInfo& lhs, const CSignature& rhs)
        {
            return lhs.Signature.GetMangledNameHash() < rhs.GetMangledNameHash();
        }

        inline bool operator()(const CSignature& lhs, const SNativeFunctionInfo& rhs)
        {
            return lhs.GetMangledNameHash() < rhs.Signature.GetMangledNameHash();
        }
    };

    bool SNativeClassExports::AddFunction(const CSignature& signature, bool is_static, NativeFunctionWrapper function)
    {
        SNativeFunctionInfo* it = std::lower_bound(NativeFunctions.begin(), NativeFunctions.end(), signature, SCompareNativeFunctionSigs());
        if (it != NativeFunctions.end() && it->Signature == signature)
            return false;
        NativeFunctions.insert(it, SNativeFunctionInfo(signature, is_static, function));
        return true;
    }

    bool SNativeClassExports::LookupFunction(const CSignature& signature, bool* is_static, NativeFunctionWrapper* function) const
    {
        SNativeFunctionInfo* it = std::lower_bound(NativeFunctions.begin(), NativeFunctions.end(), signature, SCompareNativeFunctionSigs());
        if (it != NativeFunctions.end() && it->Signature == signature)
        {
            *is_static = it->IsStatic;
            *function = it->Function;

            return true;
        }

        return false;
    }

    SNativeClassExports* LookupOrAddNativeClassExports(const char* class_name) // 114
    {
        RegisterNatives();

        ClassNativeFunctionMap::iterator it = gRegisteredNativeFunctionMap.find(class_name);
        if (it != gRegisteredNativeFunctionMap.end())
            return &it->second;

        gRegisteredNativeFunctionMap.insert(ClassNativeFunctionMap::value_type(class_name, SNativeClassExports()));
        return &gRegisteredNativeFunctionMap.find(class_name)->second;
    }

    SNativeClassExports* LookupNativeClassExports(const char* class_name)
    {
        RegisterNatives();

        ClassNativeFunctionMap::iterator it = gRegisteredNativeFunctionMap.find(class_name);
        if (it != gRegisteredNativeFunctionMap.end())
            return &it->second;

        return NULL;
    }

    void RegisterNativeFunction(const char* class_name, const CSignature& signature, bool is_static, NativeFunctionWrapper function) // 150
    {
        SNativeClassExports* exports = LookupOrAddNativeClassExports(class_name);
        if (!exports->AddFunction(signature, is_static, function))
            printf("%s.%s : function already exposed\n", class_name, signature.GetMangledName());

    }

    bool LookupNativeCallImpl(const char* class_name, const CSignature& signature, bool* is_static, NativeFunctionWrapper* function, CVector<MMString<char> >* close_matches)
    {
        SNativeClassExports* exports = LookupNativeClassExports(class_name);
        if (exports == NULL) return false;

        if (exports->LookupFunction(signature, is_static, function))
            return true;
        
        if (close_matches != NULL)
        {
            // todo: i dont really care implement it some other time
        }

        return false;
    }

    bool LookupNativeCall(const char* class_name, const CSignature& signature, bool* is_static, NativeFunctionWrapper* function) // 206
    {
        return LookupNativeCallImpl(class_name, signature, is_static, function, NULL);
    }
#else
    bool LookupNativeCall(const char* class_name, const CSignature& signature, bool* is_static, NativeFunctionWrapper* function) // 206
    {
        return false;
    }
#endif
}