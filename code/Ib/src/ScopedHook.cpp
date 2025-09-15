#include <Ib/ScopedHook.h>
using namespace Ib;

Ib::ScopedHook::ScopedHook(const char* name, uintptr_t address, const void* func) : Name(name)
{
    Handle = Hook(address, func);
}

Ib::ScopedHook::~ScopedHook()
{
    Release(Handle);
}
