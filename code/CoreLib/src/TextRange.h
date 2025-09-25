#pragma once

#include <StringUtil.h>

template <typename T>
class TextRange {
public:
    TextRange() : Begin(), End()
    {

    }

    TextRange(const T* begin) : Begin(begin), End(begin + StringLength(begin))
    {

    }

    TextRange(const T* begin, const T* end) : Begin(begin), End(end)
    {

    }
public:
    bool Valid() const
    {
        return Begin < End;
    }

    bool Empty() const
    {
        return Begin == End;
    }

    u32 Length() const
    {
        return End - Begin;
    }

    void SkipWhite();
    void SkipWhiteQ();
    
    void TrimWhite()
    {
        if (End <= Begin) return;
        while (Begin != End && IsWhiteSpace(*Begin)) Begin++;

        if (End <= Begin) return;
        while (End != Begin && IsWhiteSpace(*(End - 1))) End--;
    }

    void TrimWhiteQ();

    char GetNext()
    {
        return *Begin++;
    }
    
    T Peek() const
    {
        return *Begin;
    }

    bool Find(const T*, TextRange*) const;
    bool Find(T, TextRange*) const;
    bool FindOneOf(const T*, TextRange*) const;
    bool Equals(const TextRange& rhs) const;
    s32 Compare(const TextRange& rhs) const;
    bool StartsWith(const T*) const;
public:
    const T* Begin;
    const T* End;
};

typedef TextRange<char> TextRangeA;
typedef TextRange<wchar_t> TextRangeW;