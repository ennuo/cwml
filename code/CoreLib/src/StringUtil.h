#pragma once

#include <stdio.h>

const u32 MAX_UINT32_STRING_LENGTH = 16;
const u32 MAX_UINT64_STRING_LENGTH = 24;
const u32 MAX_FLOAT_STRING_LENGTH = 64;

size_t StringLength(const char* str);
size_t StringLength(const wchar_t* s);
size_t StringLength(const tchar_t* s);

int StringCompare(const char* a, const char* b);
int StringCompare(const wchar_t* a, const wchar_t* b);
size_t StringCompareN(const char* a, const char* b, size_t len);
int StringICompare(const char* a, const char* b);
int StringICompareN(const char* a, const char* b, size_t len);


unsigned int StringCopy(char* dst, char const* src, unsigned int size);
unsigned int StringCopy(wchar_t* dst, wchar_t const* src, unsigned int size);

size_t StringAppend(char* dst, const char* src, int dst_size);
size_t StringAppend(wchar_t* dst, const wchar_t* src, int dst_size);
size_t StringAppend(tchar_t* dst, const tchar_t* src, int dst_size);

template <typename T, u32 size>
inline size_t StringAppend(T (&dst)[size], const T* src)
{
	return StringAppend(dst, src, size);
}

/* StringUtil.h: 56 */
template <typename T, unsigned int size>
inline size_t StringCopy(T dst[size], T const* src) 
{
	return StringCopy((T*)dst, src, size);
}

inline bool IsWhiteSpace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline bool IsWhiteSpace(wchar_t c)
{
	return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

inline bool StringIsNullOrEmpty(char* str) 
{
	return str == NULL || *str == '\0';
}


// just use sprintf i dont give a shit
template <u32 size>
inline size_t FormatString(char (&dst)[size], const char* format, ...)
{
	va_list args;
	va_start(args, format);
	size_t len = vsnprintf(dst, size, format, args);
	va_end(args);
	return len;
}
