#pragma once

class c32 {
public:
    static c32 White;
public:
    inline c32()
    {
        Bits = 0;
    }

    inline c32(u32 c)
    {
        Bits = c;
    }
public:
    inline u8 GetA() { return (u8)((Bits >> 24) & 0xff); }
    inline u8 GetR() { return (u8)((Bits >> 16) & 0xff); }
    inline u8 GetG() { return (u8)((Bits >> 8) & 0xff); }
    inline u8 GetB() { return (u8)((Bits >> 0) & 0xff); }

    inline float GetAf() { return GetA() / 255.0f; }
    inline float GetRf() { return GetR() / 255.0f; }
    inline float GetGf() { return GetG() / 255.0f; }
    inline float GetBf() { return GetB() / 255.0f; }

    inline void GetRGBAf(float& r, float& g, float& b, float& a)
    {
        r = GetRf();
        g = GetGf();
        b = GetBf();
        a = GetAf();
    }

    inline v4 AsV4()
    {
        return v4(GetRf(), GetGf(), GetBf(), GetAf());
    }

    inline void GetRGBAf(float* col)
    {
        GetRGBAf(col[0], col[1], col[2], col[3]);
    }

    inline u32 AsGPUCol()
    {
        return (Bits << 8) | (Bits >> 0x18);
    }
    
	bool operator==(const c32* rhs) const { return Bits == rhs->Bits; }
	bool operator!=(const c32* rhs) const { return Bits != rhs->Bits; }
public:
    u32 Bits;
};

c32 ReplaceA(c32 c, u8 a);
c32 HalfBright(c32 c);
