#pragma once

class CAllocatorMM {
public:
    static void* Malloc(u32 size);
    static void Free(void* data);
    static void* Realloc(void* data, u32 size);
    static u32 ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof);
};

class CAllocatorMMAligned128 {
public:
    static void* Malloc(u32 size);
    static void Free(void* data);
    static void* Realloc(void* data, u32 size);
    static u32 ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof);
};
