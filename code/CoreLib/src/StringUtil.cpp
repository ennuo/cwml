#include <StringUtil.h>
#include <MMString.h>

size_t StringLength(const char* s)
{
    return strlen(s);
}

size_t StringLength(const wchar_t* s)
{
	return wcslen(s);
}

size_t StringLength(const tchar_t* s)
{
	return wcslen((const wchar_t*)s);
}

int StringCompare(const char* a, const char* b)
{
	return strcmp(a, b);
}

int StringCompare(const wchar_t* a, const wchar_t* b)
{
	return wcscmp(a, b);
}

size_t StringCompareN(const char* a, const char* b, size_t len)
{
	return strncmp(a, b, len);
}

int StringICompare(const char* a, const char* b)
{
	return strcasecmp(a, b);
}

int StringICompareN(const char* a, const char* b, size_t len)
{
	return strncasecmp(a, b, len);
}

template<typename T>
size_t StringCopy(T* dst, T const* src, unsigned int size)
{
    unsigned int num = size;
	T* ptr = (T*) src;
	while (*src && num--)
		*dst++ = *src++;
	*dst = '\0';
	return src - ptr;
}

unsigned int StringCopy(char* dst, const char* src, unsigned int size)
{
    return StringCopy<char>(dst, src, size);
}

unsigned int StringCopy(wchar_t* dst, const wchar_t* src, unsigned int size)
{
    return StringCopy<wchar_t>(dst, src, size);
}

unsigned int StringCopy(char* dst, char const* src, unsigned int size);
unsigned int StringCopy(wchar_t* dst, wchar_t const* src, unsigned int size);

u32 MultiByteStringLength_Chars(const char* str, const char* str_end)
{
	int len = 0;
	while (str != str_end)
	{
		char c = *str;

		int n = 1;
		if ((c & 0x80) == 0) n = 1;
		else if ((c & 0xe0) == 0xc0) n = 2;
		else if ((c & 0xf0) == 0xe0) n = 3;
		else if ((c & 0xf8) == 0xf0) n = 4;

		str += n;
		len++;
	}

	return len;
}

const wchar_t* MultiByteToWChar_(wchar_t* str, const char* utf8_str, const char* utf8_str_end, u32 dstlen, u32* len_out)
{
	const wchar_t* begin = str;

	while (dstlen > 1 && utf8_str != utf8_str_end)
	{
		char c = *utf8_str++;
		if ((c & 0x80) == 0) *str++ = c;
		else if ((c & 0xe0) == 0xc0)
			*str++ = (*utf8_str++ & 0x1f) << 6 | c & 0x3f;
		else if ((c & 0xf0) == 0xe0)
			*str++ = (c << 0xc) | ((*utf8_str++ & 0x3f) << 6) | (*utf8_str++ & 0x3f);
		
		dstlen--;
	}

	*str = '\0';
	
	if (len_out != NULL)
		*len_out = (str - begin) + 1;
	return begin;
}

const tchar_t* MultiByteToTChar(MMString<tchar_t>& dst, const char* src, const char* src_end)
{
	if (src_end == NULL) src_end = src + StringLength(src);
	u32 len = MultiByteStringLength_Chars(src, src_end);
	dst.resize(len + 1, L'\0');
	MultiByteToWChar_((wchar_t*)dst.c_str(), src, src_end, len + 1, &len);
	if (len != 0) len--;
	dst.resize(len, L'\0');
	return dst.c_str();
}

template <typename T>
size_t StringAppend(T* dst, const T* src, unsigned int siz) // 136
{
	T* d = dst;
	const T* s = src;

	for (size_t n = siz; n > 0 && *d != '\0'; d++, n--);

	size_t dlen = (d - dst);
	size_t n = siz - dlen;

	if (n == 0) return dlen + StringLength(src);

	while (n > 1 && *s != '\0')
	{
		*d++ = *s++;
		n--;
	}

	*d = '\0';

	return dlen + s - src;
}

size_t StringAppend(char* dst, const char* src, int dst_size) // 169
{
	return StringAppend<char>(dst, src, dst_size);
}

size_t StringAppend(wchar_t* dst, const wchar_t* src, int dst_size)
{
	return StringAppend<wchar_t>(dst, src, dst_size);
}

size_t StringAppend(tchar_t* dst, const tchar_t* src, int dst_size)
{
	return StringAppend<tchar_t>(dst, src, dst_size);
}