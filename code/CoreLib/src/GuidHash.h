#pragma once

#include <vector.h>

class CGUID {
public:
	static CGUID Zero;
public:
    inline CGUID() : guid() {}
    inline CGUID(int value) : guid(value) {}
public:
	inline void Clear() { guid = 0; }
public:
	operator int() const { return guid; }
	inline operator bool() const { return guid != 0; }
	inline bool operator !() const { return guid == 0; }
	inline int Compare(CGUID const& rhs) { return rhs.guid - guid; }
	inline bool operator==(CGUID const& rhs) const { return guid == rhs.guid; }
	inline bool operator!=(CGUID const& rhs) const { return guid != rhs.guid; }
	inline bool operator<(CGUID const& rhs) const { return guid < rhs.guid; }
	inline bool operator<=(CGUID const& rhs) const { return guid <= rhs.guid; }
	inline bool operator>(CGUID const& rhs) const { return guid > rhs.guid; }
	inline bool operator>=(CGUID const& rhs) const { return guid >= rhs.guid; }

	inline bool operator==(int rhs) const { return guid == rhs; }
	inline bool operator!=(int rhs) const { return guid != rhs; }
	inline bool operator<(int rhs) const { return guid < rhs; }
	inline bool operator<=(int rhs) const { return guid <= rhs; }
	inline bool operator>(int rhs) const { return guid > rhs; }
	inline bool operator>=(int rhs) const { return guid >= rhs; }
public:
    u32 guid;
};


class CHash { // 72
public:
	static const u32 kHashBufSize = 20;
	static const u32 kHashHexStringSize = (kHashBufSize * 2) + 1;
	static CHash Zero;
public:
	inline CHash() { Clear(); }
	CHash(const uint8_t* in, size_t len);
	CHash(const char* in, size_t len);
	CHash(const ByteArray&);
public:
	inline void* GetBuf() { return Bytes; }
public:
	void ConvertToHex(char(&)[kHashHexStringSize]) const;
public:
	inline void Clear() { memset(Bytes, 0, kHashBufSize); }
	inline int Compare(CHash const& b) const { return memcmp(Bytes, b.Bytes, kHashBufSize); }
	inline bool IsSet() const { return memcmp(Bytes, Zero.Bytes, kHashBufSize) != 0; }
public:
	inline operator bool() const { return IsSet(); }
	
	inline bool operator<(CHash const& rhs) const { return Compare(rhs) < 0; }
	inline bool operator>(CHash const& rhs) const { return Compare(rhs) > 0; }
	inline bool operator !() const { return !IsSet(); }
	inline bool operator==(CHash const& rhs) const { return Compare(rhs) == 0; }
	inline bool operator!=(CHash const& rhs) const { return Compare(rhs) != 0; }
private:
	uint8_t Bytes[kHashBufSize];
};

class StringifyHash {
public:
	inline StringifyHash(const CHash& hash) { hash.ConvertToHex(Buffer); }
	inline const char* c_str() const { return Buffer; }
private:
	char Buffer[CHash::kHashHexStringSize];
};
