#pragma once

#include <Fart.h>

const u32 FARC = 0x46415243;
const u32 FAR2 = 0x46415232;
const u32 FAR3 = 0x46415234;
const u32 FAR4 = 0x46415235;

struct Footer {
    u32 count;
    u32 magic;
};

enum EFartROState {
    EFartROState_Disabled,
    EFartROState_Enabled,
    EFartROState_Updating
};

enum ETryGetReaderResult {
    ETryGetReaderResult_False,
    ETryGetReaderResult_True,
    ETryGetReaderResult_Wait
};

class CFartRO : public CCache {
public:
class CFAT {
public:
    bool operator<(CFAT const& rhs) const { return hash.Compare(rhs.hash) < 0; }
public:
    CHash hash;
    u32 offset;
    u32 size;
};
public:
    CFartRO(const CFilePath& _fp, bool enabled, bool sec, bool want_check_cache);
    ~CFartRO();
public:
    bool GetReader(const CHash& hash, SResourceReader& out);
    void CloseCache(bool updating);
    bool OpenCache();
    bool GetSize(const CHash& hash, u32& out);
    bool IsSlow(const SResourceReader& rdr);
    bool Unlink(const CHash& hash);
    void CloseReader(SResourceReader& in, bool hashes_matched);
    bool Put(CHash&, const void*, u32);
private:
    CFAT* Find(const CHash& hash);
    bool GetReader(CFAT* f, SResourceReader& out);
    ETryGetReaderResult TryGetReader(const CHash& hash, SResourceReader& out);
    bool TryCloseCache(bool updating);
public:
    CFilePath fp;
    CRawVector<CFAT> FAT;
    FileHandle fd;
    bool Secure;
    bool Slow;
    bool WantCheckCache;
    u32 Readers;
    EFartROState State;
};

class CFartManyRO : public CCache {
public:
class CFAT : public CFartRO::CFAT {
public:
    CFartRO* Owner;
};
public:
    CFartManyRO();
    ~CFartManyRO();
public:
    CVector<CFartRO*> Farts;
    CRawVector<CFAT> FAT;
};

CFartRO* MakeROCache(const CFilePath& farc_file, bool enabled, bool secure, bool want_check_cache);