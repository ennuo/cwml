#pragma once

#include <Ib/Hook.h>

namespace Ib
{
    class ScopedHook {
    public:
        ScopedHook(const char* name, uintptr_t address, const void* func);
        ~ScopedHook();
    private:
        const char* Name;
        HookHandle Handle;
    };
};
