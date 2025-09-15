#include "GuidHash.h"
#include <sha1.h>

CGUID CGUID::Zero;
CHash CHash::Zero;

const char* HexChars = "0123456789abcdef";
void CHash::ConvertToHex(char(&strbuf)[CHash::kHashHexStringSize]) const
{
    for (int i = 0; i < CHash::kHashBufSize; ++i)
    {
        u8 b = Bytes[i];
        strbuf[i * 2] = HexChars[b >> 4];
        strbuf[(i * 2) + 1] = HexChars[b & 0xf];
    }
    
    strbuf[kHashHexStringSize - 1] = '\0';
}

CHash::CHash(const uint8_t* in, size_t len)
{
    SHA1(in, len, Bytes);
}

CHash::CHash(const char* in, size_t len)
{
    SHA1((const uint8_t*)in, len, Bytes);
}

CHash::CHash(const ByteArray& in)
{
    SHA1((const uint8_t*)in.begin(), in.size(), Bytes);
}