#pragma once

#include <stdint.h>

#ifdef VITA
    #include <taihen.h>

    struct HookHandle
    {
        tai_hook_ref_t Ref;
        SceUID UID;
    };

    #define IMAGE_BASE (0x81000000)
    #define Ib_CONTINUE(type, hook, ...) TAI_CONTINUE(type, hook, __VA_ARGS__)

    #define Ib_DeclarePort(name, type, ...) extern type (*name)(__VA_ARGS__)
#elif defined(PS3)
    typedef uint64_t HookHandle;
    #define IMAGE_BASE (0x0)
    #define Ib_CONTINUE(type, hook, ...)

    #define INVALID_HOOK_HANDLE (~0ull)

    struct Opd
    {
        void* Function;
        void* TOC;
    };

    #define Ib_DefineFunc(name, address, toc, type, ...) \
    static Opd _##name = { (void*)(address), (void*)(toc) }; \
    type(*name)(__VA_ARGS__) = (type (*)(__VA_ARGS__))&_##name;

    #define Ib_DefinePort(name, type, ...) \
    static Opd _##name = { (void*)(PORT_##name ## _ADDRESS), (void*)(PORT_##name ## _TOC) }; \
    type(*name)(__VA_ARGS__) = (type (*)(__VA_ARGS__))&_##name;

    #define Ib_DeclarePort(name, type, ...) extern type (*name)(__VA_ARGS__)
#else
    // Shouldn't be using hooks on windows, so we'll just emit
    // dummy functions that abort.

    typedef uint64_t HookHandle;
    #define IMAGE_BASE (0x0)
    #define Ib_CONTINUE(type, hook, ...)

    #define Ib_DefineFunc(name, address, toc, type, ...) type name(__VA_ARGS__)
    #define Ib_DefinePort(name, type, ...) \
    type name(__VA_ARGS__) \
    { \
        abort(); \
    } \
    
    #define Ib_DeclarePort(name, type, ...) type name(__VA_ARGS__)
#endif

#define Ib_PokeHook(address, function) Ib::Hook((uintptr_t)address, (const void*)&function)
#define Ib_ReplacePort(name, function) \
    printf("ib: replacing port %s -> %s\n", #name, #function); \
    Ib_PokeHook(PORT_##name ## _ADDRESS, function);
#define Ib_PokeCall(address, function) Ib::ReplaceCall((uintptr_t)address, (void*)&function)

namespace Ib
{
    struct InitArgs
    {
        // Pointer to executable data to be used for hooking,
        // this is only necessary on PS3.
        void* ExecutableData;

        // Pointer to an allocated EmulatorWriteCache instance,
        // this is only necessary on RPCS3.
        void* WriteCache;
    };

    void Initialize(InitArgs* args);
    void Close();

    HookHandle Hook(uintptr_t address, const void* func);
    HookHandle ReplaceCall(uintptr_t address, const void* hook);

    void Release(HookHandle handle);
}




