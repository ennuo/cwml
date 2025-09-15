#pragma once

const u32 PRNG_A = 0x02e90edd;
const u32 PRNG_B = 0x0e908fc1;
const u32 PRNG_M = 0x40000000;
const u32 PRNG_M_MASK = 0x3fffffff;
const float PRNG_INV_DENOM = 6.10388815402985E-5;

class CRandomStream {
public:
    inline CRandomStream(u32 seed) { SetSeed(seed); }
    inline void SetSeed(u32 seed) { r0 = seed; }
    u32& GetSeedForSerialisation() { return r0; }
private:
    u32 r0;
};
