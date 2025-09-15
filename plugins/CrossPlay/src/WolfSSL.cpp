#include <memory/Memory.h>
#include <sys/random_number.h>
#include <DebugLog.h>

int _fuckoff_errno;

namespace MM
{
    Ib_DefinePort(Pow, double, double, double);
    Ib_DefinePort(Log, double, double, int);
}

extern "C" int XGENRAND(unsigned char* output, unsigned int sz)
{
    return sys_get_random_number((void*)output, sz);
}

extern "C" double XPOW(double l, double r)
{
    return MM::Pow(l, r);
}

extern "C" double XLOG(double l)
{
    return MM::Log(l, 0);
}

extern "C" void* XMALLOC(size_t n, void* heap, int type)
{
    // MMLog("WolfSSL: malloc %d bytes (%d)\n", n, type);
    return MM::Malloc(n);
}

extern "C" void* XREALLOC(void *p, size_t n, void* heap, int type)
{
    return MM::Realloc(p, n);
}

extern "C" void XFREE(void *p, void* heap, int type)
{
    MM::Free(p);
}
