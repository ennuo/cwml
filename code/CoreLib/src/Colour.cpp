#include "Colour.h"

c32 c32::White(0xFFFFFFFF);

c32 ReplaceA(c32 c, u8 a)
{
    return c32(c.Bits & 0xffffff | (a << 0x18));
}

c32 HalfBright(c32 c)
{
    return c32(c.Bits >> 1 & 0x7f7f7f7f);
}

