#include <FartRO.h>
#include <algorithm>
#include <thread.h>
#include <DebugLog.h>
#include <savegame/Quick.h>

struct SCompareFAT
{
    bool operator()(CFartRO::CFAT const& lhs, CFartRO::CFAT const& rhs) const
    {
        return lhs < rhs;
    }

    bool operator()(CFartManyRO::CFAT const& lhs, CFartRO::CFAT const& rhs) const
    {
        return lhs < rhs;
    }

    bool operator()(CHash const& lhs, CFartRO::CFAT& rhs) const
    {
        return lhs < rhs.hash;
    }

    bool operator()(CFartRO::CFAT const& lhs, CHash const& rhs) const
    {
        return lhs.hash < rhs;
    }
};

CFartRO::CFartRO(const CFilePath& _fp, bool enabled, bool sec, bool want_check_cache) : CCache(fp.GetFilename()),
fp(_fp), FAT(), State(EFartROState_Disabled), Secure(sec), WantCheckCache(want_check_cache), Slow(false), fd(INVALID_FILE_HANDLE), Readers(0)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    if (enabled && !OpenCache())
        CloseCache(false);
}

CFartRO::~CFartRO()
{

}

bool CFartRO::TryCloseCache(bool updating)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    if (Readers != 0) return false;
    FAT.resize(0);
    State = updating ? EFartROState_Updating : EFartROState_Disabled;
    return true;
}

void CFartRO::CloseCache(bool updating)
{
    while (true)
    {
        if (TryCloseCache(updating)) break;
        ThreadSleep(1);
    }
}

bool CFartRO::OpenCache()
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);

    SSaveKey key;
    LocalUserID id;
    if (WantCheckCache && CheckCache(fp, key, &id) != REFLECT_OK)
    {
        MMLog("Check cache failed in CFartRO::OpenCache\n");
        return false;
    }

    FileHandle fd;
    if (!FileOpen(fp, fd, Secure ? OPEN_SECURE : OPEN_READ))
        return false;

    Footer footer;
    FileSeek(fd, -sizeof(Footer), FILE_END);
    if (FileRead(fd, &footer, sizeof(Footer)) != sizeof(Footer))
    {
        FileClose(fd);
        return false;
    }

#ifdef WIN32
    footer.count = _byteswap_ulong(footer.count);
    footer.magic = _byteswap_ulong(footer.magic);
#endif

    int revision = GetFartRevision(footer.magic);
    int npos = sizeof(Footer) + (sizeof(CFAT) * footer.count);
    if (revision >= 2) npos -= sizeof(CHash);

    if (FileSeek(fd, -npos, FILE_END) == 0)
    {
        FileClose(fd);
        return false;
    }

    FAT.resize(footer.count);
    int fat_size = footer.count * sizeof(CFAT);
    State = EFartROState_Enabled;

    if (FileRead(fd, FAT.begin(), fat_size) != fat_size)
    {
        FileClose(fd);
        return false;
    }

#ifdef WIN32
    for (int i = 0; i < FAT.size(); ++i)
    {
        CFAT& fat = FAT[i];
        fat.offset = _byteswap_ulong(fat.offset);
        fat.size = _byteswap_ulong(fat.size);
    }
#endif

    std::sort(FAT.begin(), FAT.end(), SCompareFAT());

    FileClose(fd);
    return true;
}

bool CFartRO::GetReader(const CHash& hash, SResourceReader& out)
{
    ETryGetReaderResult state;
    while (true)
    {
        state = TryGetReader(hash, out);
        if (state != ETryGetReaderResult_Wait) break;
        ThreadSleep(1);
    }

    return state == ETryGetReaderResult_True;
}

bool CFartRO::GetReader(CFAT* f, SResourceReader& out)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);

    if (fd == INVALID_FILE_HANDLE)
    {
        if (!FileOpen(fp, out.Handle, Secure ? OPEN_SECURE : OPEN_READ))
            return false;
    }
    else
    {
        out.Handle = fd;
        fd = INVALID_FILE_HANDLE;
    }

    Readers++;
    out.OriginalHash = f->hash;
    out.OwnerData = Slow;
    out.BytesRead = 0;
    out.RollingHash.Reset();
    out.Size = f->size;
    out.Offset = f->offset;

    FileSeek(out.Handle, f->offset, FILE_BEGIN);

    return true;
}

CFartRO::CFAT* CFartRO::Find(const CHash& hash)
{
    CFAT* f = std::lower_bound(FAT.begin(), FAT.end(), hash, SCompareFAT());
    return f != NULL && f->hash == hash ? f : NULL;
}

bool CFartRO::Unlink(const CHash& hash)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    CFAT* f = Find(hash);
    if (f != NULL)
    {
        FAT.erase(f);
        return true;
    }

    return false;
}

bool CFartRO::GetSize(const CHash& hash, u32& out)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    CFAT* f = Find(hash);

    if (f == NULL) return false;

    out = f->size;
    return true;
}

bool CFartRO::IsSlow(const SResourceReader& rdr)
{
    return (bool)rdr.OwnerData;
}

ETryGetReaderResult CFartRO::TryGetReader(const CHash& hash, SResourceReader& out)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);
    ETryGetReaderResult result = ETryGetReaderResult_Wait;
    if (State != EFartROState_Updating)
    {
        CFAT* f = Find(hash);
        if (f != NULL && GetReader(f, out))
            result = ETryGetReaderResult_True;
        else
            result = ETryGetReaderResult_False;
    }

    return result;
}

void CFartRO::CloseReader(SResourceReader& in, bool hashes_matched)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);

    if (fd == INVALID_FILE_HANDLE)
    {
        fd = in.Handle;
        in.Handle = INVALID_FILE_HANDLE;
    }
    else FileClose(in.Handle);
    
    if (Readers != 0) Readers--;
    if (!hashes_matched) Unlink(in.OriginalHash);
}

bool CFartRO::Put(CHash& hash_in_out, const void* bin, u32 size)
{
    return false;
}

CFartRO* MakeROCache(const CFilePath& farc_file, bool enabled, bool secure, bool want_check_cache)
{
    return new CFartRO(farc_file, enabled, secure, want_check_cache);
}