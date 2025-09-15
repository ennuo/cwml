#pragma once

#include <cstddef>

#include <vm/ConvertTypes.h>
#include <vm/ScriptContext.h>

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

    template <typename ReturnType, typename Arg1, typename Arg2>
    class CNativeFunction2 {
    public:
        template <ReturnType (&Fn)(Arg1, Arg2)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
                Arg2 b;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            SConvertScriptTypes<Arg2>::VMToNative(call.b, context, (typename SConvertScriptTypes<Arg2>::VMType&)((CallStruct*)arguments)->b);
            ReturnType value = Fn(call.a, call.b);
            ConvertReturnValue<ReturnType>(ret, context, value);
        }
    };

    class CNativeFunction0V {
    public:
        template <void (&Fn)()>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            Fn();
        }
    };

    template <typename Arg1>
    class CNativeFunction1V {
    public:
        template <void (&Fn)(Arg1)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            Fn(call.a);

            // how do we calculate argument alignment?

            // ConvertToNative<TArgumentAlignment<0u, Arg1>(*this, CScriptContext*, u8* arguments, TArgumentAlign<0u, Arg1> a)
                // SConvertScriptTypes<Arg1>::VMToNative(Arg1*, CScriptContext* u8* arguments)
            // Fn(arg1)
            // if not void
                // ConvertReturnValue<ReturnType>(void* vm_addr, CScriptContext* context, ReturnType* native)
        }
    };

    template <typename Arg1, typename Arg2>
    class CNativeFunction2V {
    public:
        template <void (&Fn)(Arg1, Arg2)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
                Arg2 b;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            SConvertScriptTypes<Arg2>::VMToNative(call.b, context, (typename SConvertScriptTypes<Arg2>::VMType&)((CallStruct*)arguments)->b);
            Fn(call.a, call.b);
        }
    };

    template <typename Arg1, typename Arg2, typename Arg3>
    class CNativeFunction3V {
    public:
        template <void (&Fn)(Arg1, Arg2, Arg3)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
                Arg2 b;
                Arg3 c;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            SConvertScriptTypes<Arg2>::VMToNative(call.b, context, (typename SConvertScriptTypes<Arg2>::VMType&)((CallStruct*)arguments)->b);
            SConvertScriptTypes<Arg3>::VMToNative(call.c, context, (typename SConvertScriptTypes<Arg3>::VMType&)((CallStruct*)arguments)->c);
            Fn(call.a, call.b, call.c);
        }
    };

    template <typename ReturnType, typename Arg1, typename Arg2, typename Arg3>
    class CNativeFunction3 {
    public:
        template <ReturnType (&Fn)(Arg1, Arg2, Arg3)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
                Arg2 b;
                Arg3 c;
            } 
            call;

            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            SConvertScriptTypes<Arg2>::VMToNative(call.b, context, (typename SConvertScriptTypes<Arg2>::VMType&)((CallStruct*)arguments)->b);
            SConvertScriptTypes<Arg3>::VMToNative(call.c, context, (typename SConvertScriptTypes<Arg3>::VMType&)((CallStruct*)arguments)->c);
            
            ReturnType value = Fn(call.a, call.b, call.c);
            ConvertReturnValue<ReturnType>(ret, context, value);
        }
    };
    
    template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    class CNativeFunction4V {
    public:
        template <void (&Fn)(Arg1, Arg2, Arg3, Arg4)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
                Arg2 b;
                Arg3 c;
                Arg4 d;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            SConvertScriptTypes<Arg2>::VMToNative(call.b, context, (typename SConvertScriptTypes<Arg2>::VMType&)((CallStruct*)arguments)->b);
            SConvertScriptTypes<Arg3>::VMToNative(call.c, context, (typename SConvertScriptTypes<Arg3>::VMType&)((CallStruct*)arguments)->c);
            SConvertScriptTypes<Arg4>::VMToNative(call.d, context, (typename SConvertScriptTypes<Arg4>::VMType&)((CallStruct*)arguments)->d);
            Fn(call.a, call.b, call.c, call.d);
        }
    };
}