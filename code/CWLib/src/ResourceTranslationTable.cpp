#include <ResourceTranslationTable.h>

struct KeyComparer
{
    bool operator()(const RTranslationTable::Index& lhs, const RTranslationTable::Index& rhs) const
    {
        return lhs.Hash < rhs.Hash;
    }

    bool operator()(u32 lhs, const RTranslationTable::Index& rhs) const
    {
        return lhs < rhs.Hash;
    }

    bool operator()(const RTranslationTable::Index& lhs, u32 rhs) const
    {
        return lhs.Hash < rhs;
    }
};

RTranslationTable::RTranslationTable(EResourceFlag flags) :
CResource(flags), Data(), IndexStart(), IndexEnd()
{

}

RTranslationTable::~RTranslationTable()
{

}

ReflectReturn RTranslationTable::Load()
{
    Data = CSR->Data;

    u32 count = *(u32*)Data.begin();
#ifdef WIN32
    count = _byteswap_ulong(count);
#endif

    IndexStart = (Index*)(Data.begin() + sizeof(u32));
    IndexEnd = IndexStart + count;

    for (Index* it = IndexStart; it != IndexEnd; ++it)
    {
#ifdef WIN32
        it->Hash = _byteswap_ulong(it->Hash);
        it->Offset = _byteswap_ulong(it->Offset);
#endif
    }

    return REFLECT_OK;
}

void RTranslationTable::Unload()
{
    Data.resize(0);
    IndexEnd = NULL;
    IndexStart = NULL;
    CResource::Unload();
}

bool RTranslationTable::GetText(u32 key, const tchar_t*& out)
{
    Index* it = std::lower_bound(IndexStart, IndexEnd, key, KeyComparer());
    if (it != IndexEnd && it->Hash == key)
    {
        out = it->Text;
        return true;
    }

    return false;
}

