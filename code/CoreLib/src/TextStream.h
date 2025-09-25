#pragma once

#include <MMString.h>
#include <StringUtil.h>

class MMOTextStreamA {
public:
    struct AsHex 
    {
        AsHex(u32 value) : Value(value) {}
        u32 Value;
    };

    struct FmtInt
    {
        const char* Format;
        u32 Value;
        FmtInt(const char* format, u32 value) : Format(format), Value(value) {}
    };
public:
    virtual ~MMOTextStreamA();
public:
    inline MMOTextStreamA& operator<<(bool v)
    {
        OutputString(v ? "true" : "false");
        return *this;
    }
    
    inline MMOTextStreamA& operator<<(char v)
    {
        char buf[2] = { v, '\0' };
        OutputString(buf);
        return *this;
    }

    inline MMOTextStreamA& operator<<(wchar_t v)
    {
        char buf[32];
        FormatString(buf, "%C", v);
        OutputString(buf);
        return *this;

    }
    
    inline MMOTextStreamA& operator<<(int v)
    {
        char buf[32];
        FormatString(buf, "%d", v);
        OutputString(buf);
        return *this;
    }

    MMOTextStreamA& operator<<(s64);
    
    inline MMOTextStreamA& operator<<(u32 v)
    {
        char buf[32];
        FormatString(buf, "%i", v);
        OutputString(buf);
        return *this;
    }

    MMOTextStreamA& operator<<(u64);
    
    inline MMOTextStreamA& operator<<(float v)
    {
        char buf[256];
        FormatString(buf, "%f", v);
        OutputString(buf);
        return *this;
    }
    
    MMOTextStreamA& operator<<(double);
    MMOTextStreamA& operator<<(AsHex);
    MMOTextStreamA& operator<<(FmtInt);
    
    inline MMOTextStreamA& operator<<(const char* v)
    {
        OutputString(v);
        return *this;
    }

    inline MMOTextStreamA& operator<<(const wchar_t* v)
    {
        OutputString(v);
        return *this;
    }

    MMOTextStreamA& operator<<(const tchar_t*);
    
    inline MMOTextStreamA& operator<<(const MMString<char>& v)
    {
        OutputString(v.c_str());
        return *this;
    }

    inline MMOTextStreamA& operator<<(const MMString<wchar_t>& v)
    {
        OutputString(v.c_str());
        return *this;
    }

    MMOTextStreamA& operator<<(const MMString<tchar_t>&);
public:
    virtual void OutputData(const void* data, u32 length) = 0;
private:
    virtual void OutputString(const char* val) = 0;
    virtual void OutputString(const wchar_t* val) = 0;
    virtual void OutputString(const tchar_t* val) = 0;
};

class MMOTextStreamString : public MMOTextStreamA {
public:
    void OutputData(const void* data, u32 length) {}
    void OutputString(const char* val) { Result += val; }

    void OutputString(const wchar_t* val)
    {
        // WCharToMultiByteAppend(Result, val, 0);
    }

    void OutputString(const tchar_t* val)
    {
        // TCharToMultiByteAppend(Result, val, 0);
    }

    inline const char* CStr() const { return Result.c_str(); }
    inline void Clear() { Result.clear(); }
private:
    MMString<char> Result;
};