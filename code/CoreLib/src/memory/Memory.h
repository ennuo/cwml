#pragma once

class CReservedMemory {
public:
    CReservedMemory(u32 size, u32 align);
    void Init(void* data, u32 size, const char* name);
public:
    inline void* GetPtr() const { return Data; }
    inline u32 GetSize() const { return Size; }
    inline u32 GetAlign() const { return Align; }
#ifndef LBP1
    inline u32 GetContainer() const { return Container; }
#endif
private:
    void* Data;
    u32 Size;
    u32 Align;
#ifndef LBP1
    u32 Container;
#endif
};

namespace MM
{
    void* Malloc(u32 size);
    void Free(void* data);
    void* Realloc(void* data, u32 size);
    void* AlignedMalloc(u32 size, u32 align);
    void AlignedFree(void* data);
}

class CSlabAlloc {
public:
    void* Data;
    CReservedMemory AlignmentPaddingPre;
    CReservedMemory HostVideo;
    CReservedMemory GFX;
    CReservedMemory SmallGfx;
    CReservedMemory RenderTargets;
    CReservedMemory ScratchPad;
    CReservedMemory ConvexQuery;
    CReservedMemory Constraints;
    CReservedMemory BucketMem[3];
    CReservedMemory PoolMem[13];
    CReservedMemory AlignmentPaddingPost;
    /// ... blah blah
};

extern CSlabAlloc gSlabAlloc;