
#include "memory/Memory.h"

CReservedMemory::CReservedMemory(u32 size, u32 align)
{
#ifndef LBP1
    Container = ~0u;
#endif
    Data = NULL;
    Size = size;
    Align = align;
}

void CReservedMemory::Init(void* data, u32 size, const char* name)
{
    Data = data;
    Size = size;
}

#ifdef WIN32
namespace MM
{
    void* Malloc(u32 size)
    {
        return malloc(size);
    }
    
    void Free(void* data)
    {
        free(data);
    }

    void* Realloc(void* data, u32 size)
    {
        return realloc(data, size);
    }

    void* AlignedMalloc(u32 size, u32 align)
    {
        return _aligned_malloc(size, align);
    }

    void AlignedFree(void* data)
    {
        _aligned_free(data);
    }
}
#else

#ifdef LBP1

class CAllocatorBucket
{

};

Ib_DefinePort(CAllocatorBucket_Malloc, void*, CAllocatorBucket*, u32 size);
Ib_DefinePort(CAllocatorBucket_Realloc, void*, CAllocatorBucket*, void* data, u32 size);
Ib_DefinePort(CAllocatorBucket_AlignedMalloc, void*, CAllocatorBucket*, u32 size, u32 align);
Ib_DefinePort(CAllocatorBucket_AlignedRealloc, void*, CAllocatorBucket*, void* data, u32 size, u32 align);
Ib_DefinePort(CAllocatorBucket_Free, void, CAllocatorBucket*, void* data);

// LBP1 defines unique allocator buckets, but none of the allocation functions
// actually use them at all, so just going to pass a NULL pointer instead
// of grabbing the proper global variables.
namespace MM
{
    void* Malloc(u32 size)
    {
        return CAllocatorBucket_Malloc(NULL, size);
    }

    void Free(void* data)
    {
        CAllocatorBucket_Free(NULL, data);
    }

    void* Realloc(void* data, u32 size)
    {
        return CAllocatorBucket_Realloc(NULL, data, size);
    }

    void* AlignedMalloc(u32 size, u32 align)
    {
        return CAllocatorBucket_AlignedMalloc(NULL, size, align);
    }

    void AlignedFree(void* data)
    {
        // should be aligned free?
        return CAllocatorBucket_Free(NULL, data);
    }
}


#else

#include <memory/Bucket.h>

extern CSlabAlloc gSlabAlloc;

Ib_DefinePort(CSlabAlloc_Malloc, void*, CSlabAlloc&, int bucket, u32 size);
Ib_DefinePort(CSlabAlloc_Realloc, void*, CSlabAlloc&, int bucket, void* data, u32 size);
Ib_DefinePort(CSlabAlloc_AlignedMalloc, void*, CSlabAlloc&, int bucket, u32 size, u32 align);
Ib_DefinePort(CSlabAlloc_AlignedRealloc, void*, CSlabAlloc&, int bucket, void* data, u32 size, u32 align);
Ib_DefinePort(CSlabAlloc_Free, void, CSlabAlloc&, int bucket, void* data);

namespace MM
{
    void* Malloc(u32 size)
    {
        return CSlabAlloc_Malloc(gSlabAlloc, MEM_MAIN, size);
    }

    void Free(void* data)
    {
        CSlabAlloc_Free(gSlabAlloc, MEM_MAIN, data);
    }

    void* Realloc(void* data, u32 size)
    {
        return CSlabAlloc_Realloc(gSlabAlloc, MEM_MAIN, data, size);
    }

    void* AlignedMalloc(u32 size, u32 align)
    {
        return CSlabAlloc_AlignedMalloc(gSlabAlloc, MEM_MAIN, size, align);
    }

    void AlignedFree(void* data)
    {
        CSlabAlloc_Free(gSlabAlloc, MEM_MAIN, data);
    }
}

#endif

#endif // WIN32

void* operator new(size_t sz) { return MM::Malloc(sz); }
void* operator new(size_t sz, size_t align) { return MM::AlignedMalloc(sz, align); }
void* operator new[](size_t sz) { return ::operator new(sz); }
void* operator new[](size_t sz, size_t align) { return ::operator new(sz, align); }
// void* operator new(size_t sz, void* p) { return p; }
void operator delete(void* p) { MM::Free(p); }
void operator delete[](void* p) { MM::Free(p); }