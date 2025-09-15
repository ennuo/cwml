#pragma once

#include <vector.h>
#include <filepath.h>
#include <GuidHash.h>
#include <sha1.h>
#include <CritSec.h>
#include <ResourceDescriptor.h>

enum CacheType {
    CT_READONLY,
    
    CT_SYNC, // this is technically CT_ACCEL, but we use it for the r/w sync cache instead, since its unused in basically every game
    
    CT_TEMP,
    CT_DOWNLOAD,
    CT_FSB,
    CT_LEVEL,
    CT_LITTLEFARTBUILD,
    CT_SAVEGAME_FIRST,
    CT_SAVEGAME_LAST = 39,
    CT_COUNT
};

struct SResourceReader;

class CCache { // fart.h: 18
public:
    CCache(const char* name);
public:
    virtual ~CCache() = 0;
    virtual bool IsSlow(const SResourceReader&) = 0;
    virtual bool GetReader(const CHash&, SResourceReader&) = 0;
    virtual void CloseReader(SResourceReader& in, bool hashes_matched) = 0;
    virtual bool Unlink(const CHash&) = 0;
    virtual bool GetSize(const CHash&, u32&) = 0;
    virtual bool Put(CHash& hash_in_out, const void* bin, u32 size) = 0;
public:
    CCriticalSec Mutex;
};

class SResourceReader { // fart.h: 37
public:
    SResourceReader() 
    { 
        memset(this, 0, sizeof(SResourceReader)); 
        RollingHash.Reset();
        Handle = INVALID_FILE_HANDLE;
    }

    ~SResourceReader()
    {
        if (Owner != NULL)
            Owner->CloseReader(*this, true);
    }
public:
    CCache* Owner;
    FileHandle Handle;
    u32 Offset;
    u32 Size;
    s32 BytesRead;
    u32 OwnerData;
    CSHA1Context RollingHash;
    CHash OriginalHash;
};

class CQueef { // fart.h: 61
enum { buffer_size = 32768 };
private:
    u8 Buffer[buffer_size];
    SResourceReader Reader;
    bool Used;
    FileHandle Writer;
    u32 Size;
};

u32 GetFartRevision(u32 magic);
CCache* MakeLazyCache(const char* name, const CFilePath& basedir, u32 size, u32 maxfiles);
bool GetResourceReader(const CResourceDescriptorBase& desc, SResourceReader& out, CFilePath& loose_path);
bool GetResourceReader(const CHash& hash, SResourceReader& out);

bool FileLoad(SResourceReader& h, ByteArray& out);
u64 FileRead(SResourceReader& h, void* out, u64 count);
u64 FileSize(SResourceReader& h);
bool FileClose(SResourceReader& h);

extern CCache* gCaches[CT_COUNT];

bool InitCaches();
void CloseCaches();