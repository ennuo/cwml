#pragma once

#include <cstddef>

#include <vm/VMTypes.h>
#include <vm/ConvertTypes.h>
#include <vm/ScriptContext.h>
#include <vm/NativeFunction.h>

namespace NVirtualMachine
{
    template<typename ReturnType>
    void ConvertReturnValue(void*& vm_addr, CScriptContext* context, ReturnType& native)
    {
        SConvertScriptTypes<ReturnType>::NativeToVM
        (
            *((typename SConvertScriptTypes<ReturnType>::VMType*&) vm_addr), 
            context, 
            (typename SConvertScriptTypes<ReturnType>::NativeType&) native
        );
    }

    template <typename ReturnType>
    class CNativeFunction0 {
    public:
        static CSignature GetStaticSignature(const char* name)
        {
            CSignature sig;
            sig.MakeName(name);
            return sig;
        }

        static CSignature GetNonStaticSignature(const char* name)
        {
            CSignature sig;
            sig.MakeName(name);
            return sig;
        }

        template <ReturnType (&Fn)()>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            ReturnType value = Fn();
            ConvertReturnValue<ReturnType>(ret, context, value);
        }
    };

    template <typename ReturnType, typename Arg1>
    class CNativeFunction1 {
    public:
        template <ReturnType (&Fn)(Arg1)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            ReturnType value = Fn(call.a);
            ConvertReturnValue<ReturnType>(ret, context, value);
        }
    };

    class CNativeFunction0V {
    public:
        static CSignature GetStaticSignature(const char* name)
        {
            CSignature sig;
            sig.MakeName(name);
            return sig;
        }

        static CSignature GetNonStaticSignature(const char* name)
        {
            CSignature sig;
            sig.MakeName(name);
            return sig;
        }


        template <void (&Fn)()>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            Fn();
        }
    };

    template <typename Arg1>
    class CNativeFunction1V {
    public:
        static CSignature GetStaticSignature(const char* name)
        {
            CSignature sig;
            sig.MakeName(name, Arg<Arg1>().GetName());
            return sig;
        }

        static CSignature GetNonStaticSignature(const char* name)
        {
            CSignature sig;
            sig.MakeName(name, Arg<Arg1>().GetName());
            return sig;
        }

        template <void (&Fn)(Arg1)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            TArgumentAlignment<0, Arg1> a0;
            Fn(ConvertToNative<Arg1>(context, arguments, a0));
        }
    };

    CNativeFunction0V GetCallStruct(void(*func_ptr)())  { return CNativeFunction0V(); }

    template <typename Arg1>
    CNativeFunction1V<Arg1> GetCallStruct(void(*func_ptr)(Arg1)) { return CNativeFunction1V<Arg1>(); }

    template <typename ReturnType>
    CNativeFunction0<ReturnType> GetCallStruct(ReturnType(*func_ptr)()) { return CNativeFunction0<ReturnType>(); }


    #define REGISTER_NATIVE_FUNCTION(class_name, function) \
        NVirtualMachine::RegisterNativeFunction(class_name, NVirtualMachine::GetCallStruct(&function).GetNonStaticSignature(#function), false, NVirtualMachine::GetCallStruct(&function).Call<function>);

    #define REGISTER_NATIVE_FUNCTION_STATIC(class_name, function) \
        NVirtualMachine::RegisterNativeFunction(class_name, NVirtualMachine::GetCallStruct(&function).GetStaticSignature(#function), true, NVirtualMachine::GetCallStruct(&function).Call<function>);
}