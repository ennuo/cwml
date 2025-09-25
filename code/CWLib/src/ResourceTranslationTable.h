#pragma once

#include <Resource.h>
#include <vector.h>
#include <TextRange.h>


class RTranslationTable : public CResource {
public:
    struct Index
    {
        u32 Hash;
        union
        {
            u32 Offset;
            tchar_t* Text;
        };
    };
public:
    RTranslationTable(EResourceFlag);
    ~RTranslationTable();
public:
    bool GetText(u32 key, const tchar_t*& out);
    virtual ReflectReturn Load();
    virtual void Unload();
public:
    ByteArray Data;
    Index* IndexStart;
    Index* IndexEnd;
};

u32 HashTextRangeA(TextRangeA& r);
u32 HashTextRangeW(TextRangeW& r);