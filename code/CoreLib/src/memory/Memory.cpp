
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


namespace SCEA { namespace LibMalloc {

    typedef uint32_t MemSize;
    typedef uint32_t MemFlags;

    const MemFlags kMemFlagsDefault = 0;

    enum MemAlign
    {
        kMemAlignDefault            = 0,
        kMemAlignShift              = 0,
        kMemAlignMask               = (0x1fU << kMemAlignShift),
        kMemAlign8                  = (0x03U << kMemAlignShift),
        kMemAlign16                 = (0x04U << kMemAlignShift),
        kMemAlign32                 = (0x05U << kMemAlignShift),
        kMemAlign64                 = (0x06U << kMemAlignShift),
        kMemAlign128                = (0x07U << kMemAlignShift),
        kMemAlign256                = (0x08U << kMemAlignShift),
        kMemAlign512                = (0x09U << kMemAlignShift),
        kMemAlign1k                 = (0x0aU << kMemAlignShift),
        kMemAlign2k                 = (0x0bU << kMemAlignShift),
        kMemAlign4k                 = (0x0cU << kMemAlignShift),
        kMemAlign8k                 = (0x0dU << kMemAlignShift),
        kMemAlign16k                = (0x0eU << kMemAlignShift),
        kMemAlign32k                = (0x0fU << kMemAlignShift),
        kMemAlign64k                = (0x10U << kMemAlignShift),
        kMemAlign128k               = (0x11U << kMemAlignShift),
        kMemAlign256k               = (0x12U << kMemAlignShift),
        kMemAlign512k               = (0x13U << kMemAlignShift),
        kMemAlign1Meg               = (0x14U << kMemAlignShift),
        kMemAlignVector             = kMemAlign16,
        kMemAlignCacheLine          = kMemAlign128
    };

    class AllocatorBase {
    public:
        virtual void* Allocate(MemSize size, MemFlags flags, const char* pFile = NULL, int line = 0) = 0;
        virtual void Deallocate(void* pMemory, MemFlags flags, const char* pFile = NULL, int line = 0) = 0;
        virtual void* Reallocate(void* pMemory, MemSize newSize, MemFlags flags, const char* pFile = NULL, int line = 0) = 0;
    };
}}

using namespace SCEA::LibMalloc;

#include <DebugLog.h>

class CAllocatorBucket {
public:

#ifdef USE_SCEA_LIBMALLOC
    void* Malloc(u32 size)
    {
        return Allocator->Allocate(size, kMemFlagsDefault);
    }

    void Free(void* data)
    {
        if (data != NULL)
            Allocator->Deallocate(data, kMemFlagsDefault);
    }

    void* Realloc(void* data, u32 size)
    {
        if (data != NULL)
            return Allocator->Reallocate(data, size, kMemFlagsDefault);
        return Allocator->Allocate(size, kMemFlagsDefault);
    }

    void* AlignedMalloc(u32 size, u32 align)
    {
        return Allocator->Allocate(size, kMemFlagsDefault);
    }

    void AlignedFree(void* data)
    {
        if (data != NULL)
            Allocator->Deallocate(data, kMemFlagsDefault);
    }
    
    void* AlignedRealloc(void* data, u32 size, u32 align)
    {
        if (data != NULL)
            return Allocator->Reallocate(data, size, kMemFlagsDefault);
        return Allocator->Allocate(size, kMemFlagsDefault);
    }
#endif
private:
    u64 Timer[6];
    AllocatorBase* Allocator;
    u8 Type;
};

extern CAllocatorBucket gOtherBucket;
extern CAllocatorBucket gVectorBucket;

namespace MM
{
#ifdef USE_SCEA_LIBMALLOC
    void* Malloc(u32 size)
    {
        return gOtherBucket.Malloc(size);
    }

    void Free(void* data)
    {
        gOtherBucket.Free(data);
    }

    void* Realloc(void* data, u32 size)
    {
        return gOtherBucket.Realloc(data, size);
    }

    void* AlignedMalloc(u32 size, u32 align)
    {
        return gOtherBucket.AlignedMalloc(size, align);
    }

    void AlignedFree(void* data)
    {
        gOtherBucket.AlignedFree(data);
    }
#else
    // LBP1 defines unique allocator buckets, but none of the allocation functions
    // actually use them at all, so just going to pass a NULL pointer instead
    // of grabbing the proper global variables.

    Ib_DefinePort(CAllocatorBucket_Malloc, void*, CAllocatorBucket*, u32 size);
    Ib_DefinePort(CAllocatorBucket_Realloc, void*, CAllocatorBucket*, void* data, u32 size);
    Ib_DefinePort(CAllocatorBucket_AlignedMalloc, void*, CAllocatorBucket*, u32 size, u32 align);
    Ib_DefinePort(CAllocatorBucket_AlignedRealloc, void*, CAllocatorBucket*, void* data, u32 size, u32 align);
    Ib_DefinePort(CAllocatorBucket_Free, void, CAllocatorBucket*, void* data);

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
#endif
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