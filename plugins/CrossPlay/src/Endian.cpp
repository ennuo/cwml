#include <Endian.h>

u16 swap16(u16 v) { return (v >> 8) | (v << 8); }
u32 swap32(u32 v)
{
    return
        (v >> 24) |
        ((v<<8) & 0x00FF0000) |
        ((v>>8) & 0x0000FF00) |
        (v << 24);
}

u64 swap64(u64 x)
{
    x = (x & 0x00000000FFFFFFFFull) << 32 | (x & 0xFFFFFFFF00000000ull) >> 32;
    x = (x & 0x0000FFFF0000FFFFull) << 16 | (x & 0xFFFF0000FFFF0000ull) >> 16;
    x = (x & 0x00FF00FF00FF00FFull) << 8  | (x & 0xFF00FF00FF00FF00ull) >> 8;
    return x;
}
