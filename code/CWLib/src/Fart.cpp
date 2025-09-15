#include <Fart.h>
#include <FartRO.h>
#include <filepath.h>
#include <Directory.h>
#include <System.h>
#include <DebugLog.h>
#include <GuidHashMap.h>

#ifdef WIN32
    CCache* gCaches[CT_COUNT];
#endif

const char* gFarcDataFilename = "data.farc";

CCache::CCache(const char* name) : Mutex(name)
{
    
}

CCache::~CCache()
{
    
}

bool GetResourceReader(const CHash& hash, SResourceReader& out)
{
    for (int i = 0; i < CT_COUNT; ++i)
    {
        CCache* cache = gCaches[i];
        if (cache != NULL && cache->GetReader(hash, out))
            return true;
    }

    return false;
}

bool GetResourceReader(const CResourceDescriptorBase& desc, SResourceReader& out, CFilePath& loose_path)
{
    CHash hash = desc.LatestHash();
    if (hash && GetResourceReader(hash, out))
        return true;
    
    return false;
}

bool FileClose(SResourceReader& h)
{
    if (h.Handle == INVALID_FILE_HANDLE) return true;

    bool hashes_match = true;

    if (h.BytesRead > 0 && h.BytesRead == h.Size && h.OriginalHash)
    {
        CHash hash;
        if (h.RollingHash.Result((uint8_t*)&hash) == SHA1_SUCCESS && hash != h.OriginalHash && h.Owner != NULL)
        {
            MMLogCh(DC_PLAYER_PROFILE, "Hash mismatch: original: %s, rolling: %s, handle: %d\n",
                StringifyHash(h.OriginalHash).c_str(),
                StringifyHash(hash).c_str(),
                (int)h.Handle
            );

            hashes_match = false;
        }
    }

    if (h.Owner != NULL) h.Owner->CloseReader(h, true);
    FileClose(h.Handle);
    h.Handle = INVALID_FILE_HANDLE;

    return hashes_match;
}

u64 FileSize(SResourceReader& h)
{
    return h.Size;
}

u64 FileRead(SResourceReader& h, void* out, u64 count)
{
    u64 n = FileRead(h.Handle, out, count);
    if (h.BytesRead != -1)
    {
        h.BytesRead += n;
        h.RollingHash.AddData((const uint8_t*)out, count);
    }

    return n;
}

bool FileLoad(SResourceReader& h, ByteArray& out)
{
    u64 count = FileSize(h);
    u64 n = 0;

    if (out.try_resize(count))
        n = FileRead(h, out.begin(), count);

    return FileClose(h) && n == count;
}

void LoadPatches()
{

}

bool InitCaches()
{
    CFilePath syscache(FPR_SYSCACHE, "output/");
    DirectoryCreate(syscache);
    gCaches[CT_TEMP] = MakeLazyCache("temp", syscache, 0x40000000, 0x1000);

    // CFilePath fart(FPR_BLURAY, gFarcDataFilename);
    // temp hack temp hack temp hack!!
    CFilePath fart(FPR_BLURAY, "patch.sdat");
    if (FileExists(fart))
        gCaches[CT_READONLY] = MakeROCache(fart, true, false, false);
    
    return true;
}

void CloseCaches()
{
    for (int i = 0; i < CT_COUNT; ++i)
    {
        CCache*& cache = gCaches[i];
        if (cache != NULL)
            delete cache;
        cache = NULL;
    }
}