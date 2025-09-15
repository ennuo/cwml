#pragma once

#include <MMString.h>

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
    MMOTextStreamA& operator<<(bool);
    MMOTextStreamA& operator<<(char);
    MMOTextStreamA& operator<<(wchar_t);
    MMOTextStreamA& operator<<(int);
    MMOTextStreamA& operator<<(s64);
    MMOTextStreamA& operator<<(u32);
    MMOTextStreamA& operator<<(u64);
    MMOTextStreamA& operator<<(float);
    MMOTextStreamA& operator<<(double);
    MMOTextStreamA& operator<<(AsHex);
    MMOTextStreamA& operator<<(FmtInt);
    MMOTextStreamA& operator<<(const char*);
    MMOTextStreamA& operator<<(const wchar_t*);
    MMOTextStreamA& operator<<(const tchar_t*);
    MMOTextStreamA& operator<<(const MMString<char>&);
    MMOTextStreamA& operator<(const MMString<wchar_t>&);
    MMOTextStreamA& operator<<(const MMString<tchar_t>&);
public:
    virtual void OutputData(const void*, u32);
private:
    virtual void OutputString(const char*);
    virtual void OutputString(const wchar_t*);
    virtual void OutputString(const tchar_t*);
};