#pragma once

inline u32 RoundUpPow2(u32 x, u32 alignment)
{
    return (alignment + x) - 1 & -alignment;
}

inline s32 ZigZagInt(s32 h)
{
    return (u32)(h << 1) ^ (h >> 31);
}

inline s32 UnZigZagInt(s32 h)
{
    return (s32)(h >> 1) ^ (u32)(-(s32)(h & 1));
}
