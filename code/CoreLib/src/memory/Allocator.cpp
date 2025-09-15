#include "memory/Memory.h"
#include "memory/Allocator.h"

void* CAllocatorMM::Malloc(u32 size) { return MM::Malloc(size); }
void CAllocatorMM::Free(void* data) { MM::Free(data); }
void* CAllocatorMM::Realloc(void* data, u32 size) { return MM::Realloc(data, size); }

// u32 CAllocatorMM::ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof) 
// {
// 	u32 double_size = old_max_size * 2;
// 	u32 min_size = double_size;

// 	if (item_sizeof != 0)
// 	{
// 		min_size = 32 / item_sizeof;
// 		if (min_size < double_size)
// 			min_size = double_size;
// 	}

// 	return new_size > min_size ? new_size : min_size;
// }


u32 CAllocatorMM::ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof) 
{
	u64 uVar1 = ((u64)(s32)old_max_size & 0x7fffffffU) * 2;
	if (item_sizeof != 0) {
		uVar1 = uVar1 - (-(u64)(uVar1 < 0x20 / (u64)item_sizeof) &
                    uVar1 - 0x20 / (u64)item_sizeof);
	}
	return (s32)uVar1 -
         ((s32)uVar1 - new_size &
         -(u32)((uVar1 & 0xffffffff) < ((u64)(s32)new_size & 0xffffffffU)));
}

void* CAllocatorMMAligned128::Malloc(u32 size) { return MM::Malloc(size); }
void CAllocatorMMAligned128::Free(void* data) { MM::Free(data); }
void* CAllocatorMMAligned128::Realloc(void* data, u32 size) { return MM::Realloc(data, size); }

u32 CAllocatorMMAligned128::ResizePolicy(u32 old_max_size, u32 new_size, u32 item_sizeof) 
{
	u64 uVar1 = ((u64)(s32)old_max_size & 0x7fffffffU) * 2;
	if (item_sizeof != 0) {
		uVar1 = uVar1 - (-(u64)(uVar1 < 0x20 / (u64)item_sizeof) &
                    uVar1 - 0x20 / (u64)item_sizeof);
	}
	return (s32)uVar1 -
         ((s32)uVar1 - new_size &
         -(u32)((uVar1 & 0xffffffff) < ((u64)(s32)new_size & 0xffffffffU)));
}