#include <Fart.h>
#include <Directory.h>
#include <DebugLog.h>

class CFart : public CCache {
public:
    static const u32 kMagic = 0xf0feb0bb;
class FileEntry {
public:
    FileEntry() 
    {
        Lock = 0;
    }

    u32 End()
    {
        return Offset + Size;
    }

    bool Overlaps(u32 o, u32 s)
    {
        return o >= Offset && (o + s) < (Offset + Size);
    }
public:
    u32 Magic;
    u32 Offset;
    u32 Size;
    u32 Sequence;
    u32 Lock;
    CHash Hash;
};
public:
    CFart(const char* name, const CFilePath& dir, u32 size, u32 maxfiles) :
    CCache(name), FullPath(), FAT(), BaseDir(dir), _CacheFile(INVALID_FILE_HANDLE), fd_read_cache(INVALID_FILE_HANDLE)
    {
        FullPath = BaseDir;
        FullPath.Append(name);
        DirectoryCreate(FullPath);
        OpenCache(FullPath, size, maxfiles);
    }

    ~CFart()
    {
        FileClose(_CacheFile);
        FileClose(fd_read_cache);
    }
public:
    bool RewriteFAT(u32 idx, u32 magic)
    {
        if (idx >= FAT.size() || FAT[idx].Magic == magic) return false;

        u32 ofs = idx * sizeof(FileEntry);
        if (FileSeek(_CacheFile, ofs, FILE_BEGIN) != ofs)
            return false;
        
        FAT[idx].Magic = magic;

#ifdef WIN32
        FileEntry f = FAT[idx];
        f.Magic = _byteswap_ulong(f.Magic);
        f.Offset = _byteswap_ulong(f.Offset);
        f.Size = _byteswap_ulong(f.Size);
        f.Sequence = _byteswap_ulong(f.Sequence);
        f.Lock = _byteswap_ulong(f.Lock);

        FileWrite(_CacheFile, &f, sizeof(FileEntry));
#else
        FileWrite(_CacheFile, FAT.begin() + idx, sizeof(FileEntry));
#endif

        return true;
    }

    u32 RemoveFilesThatOverlap(u32& ofs, u32 size)
    {
        int fseq = -1;
        int fblk = -1;

        restart:
        for (int i = 0; i < FAT.size(); ++i)
        {
            FileEntry& f = FAT[i];
            if (f.Magic == kMagic)
            {
                if (f.Overlaps(ofs + CacheFATSize, size))
                {
                    if (f.Lock != 0)
                    {
                        ofs = f.End();
                        goto restart;
                    }
                }
                else if (Sequence < fseq && f.Lock == 0)
                {
                    fseq = f.Sequence;
                    fblk = i;

                    continue;
                }
            }

            RewriteFAT(i, 0);

            if (fseq == 0)
            {
                if (fblk == -1)
                {
                    fblk = i + 1;
                }
            }
            else
            {
                fseq = 0;
                fblk = i;
            }
        }

        return fblk;
    }

    bool IsSlow(const SResourceReader& rdr) { return false; }

    bool Put(CHash& hash_in_out, const void* bin, u32 size)
    {
        CCSLock lock(&Mutex, __FILE__, __LINE__);
        if (CacheSizeBytes - CacheFATSize <= size)
        {
            MMLogCh(DC_RESOURCE, "FartLazy: failed put, size to large\n");
            return false;
        }

        if (!!hash_in_out)
        {
            u32 idx = FindFileByHash(hash_in_out);
            if (idx != ~0ul) return true;

            if (CHash((const uint8_t*)bin, size) != hash_in_out)
                return false;
        }

        u32 offset = MAX(WriteHead, CacheFATSize);
        u32 idx = RemoveFilesThatOverlap(offset, size);

        if (idx == ~0ul)
        {
            MMLogCh(DC_RESOURCE, "Remove files that overlap returned -1\n");
            return false;
        }

        RewriteFAT(idx, 0);

        if (FileSeek(_CacheFile, offset, FILE_BEGIN) != offset)
        {
            MMLogCh(DC_RESOURCE, "FartLazy: seek failed");
            return false;
        }

        if (FileWrite(_CacheFile, bin, size) != size)
        {
            MMLogCh(DC_RESOURCE, "FartLazy: wrote the wrong number of bytes\n");
            return false;
        }

        if (!hash_in_out)
            hash_in_out = CHash((const uint8_t*)bin, size);

        FileEntry& f = FAT[idx];
        f.Offset = offset;
        f.Size = size;
        f.Sequence = ++Sequence;
        f.Hash = hash_in_out;

        RewriteFAT(idx, kMagic);
        WriteHead = offset + size;
        FileSync(_CacheFile);

        if (fd_read_cache != INVALID_FILE_HANDLE)
        {
            MMLogCh(DC_RESOURCE, "FartLazy: closing cached read fd after a sync\n");
            FileClose(fd_read_cache);
            fd_read_cache = INVALID_FILE_HANDLE;
        }

        return true;
    }

    bool GetSize(const CHash& hash, u32& out)
    {
        CCSLock lock(&Mutex, __FILE__, __LINE__);
        u32 idx = FindFileByHash(hash);
        if (idx != ~0ul)
        {
            out = FAT[idx].Size;
            return true;
        }

        return false;
    }

    bool GetReader(const CHash& hash, SResourceReader& out)
    {
        CCSLock lock(&Mutex, __FILE__, __LINE__);
        u32 idx = FindFileByHash(hash);
        if (idx == ~0ul) return false;

        if (fd_read_cache == INVALID_FILE_HANDLE)
        {
            if (!FileOpen(FullPath, out.Handle, OPEN_READ))
                return false;
        }
        else
        {
            out.Handle = fd_read_cache;
            fd_read_cache = INVALID_FILE_HANDLE;
        }

        FileEntry& f = FAT[idx];
        out.Size = f.Size;
        out.Offset = f.Offset;
        FileSeek(out.Handle, out.Offset, FILE_BEGIN);
        out.Owner = this;
        f.Lock++;
        out.OwnerData = idx;
        out.OriginalHash = hash;
        out.BytesRead = 0;
        out.RollingHash.Reset();

        return true;
    }

    void CloseReader(SResourceReader& in, bool hashes_matched)
    {
        CCSLock lock(&Mutex, __FILE__, __LINE__);
        if (fd_read_cache == INVALID_FILE_HANDLE)
        {
            fd_read_cache = in.Handle;
            fd_read_cache = INVALID_FILE_HANDLE;
        }
        else
        {
            FileClose(in.Handle);
        }

        u32 idx = in.OwnerData;
        if (idx < FAT.size() && FAT[idx].Lock != 0)
            FAT[idx].Lock--;
        
        if (!hashes_matched)
        {
            MMLogCh(DC_RESOURCE, 
                "***************** ARGH corrupt/hash mismatching file in lazy fart, removing from fat. OriginalHash: %s\n",
                StringifyHash(in.OriginalHash).c_str()
            );

            Unlink(in.OriginalHash);
        }
    }

    bool Unlink(const CHash& hash)
    {
        CCSLock lock(&Mutex, __FILE__, __LINE__);
        u32 idx = FindFileByHash(hash);
        if (idx == ~0ul) return false;
        FileEntry& f = FAT[idx];
        return RewriteFAT(idx, 0);
    }

    void OpenCache(const CFilePath& filename, u32 size, u32 maxfiles)
    {
        _CacheFile = INVALID_FILE_HANDLE;
        FileResizeNoZeroFill(filename, size);

        CacheFATSize = maxfiles * sizeof(FileEntry);
        CacheSizeBytes = size;
        FAT.resize(maxfiles);

        if (!FileOpen(filename, _CacheFile, OPEN_RDWR) || (FileSize(filename) < size && !FileResize(_CacheFile, size)))
            return;

        Sequence = 0;
        WriteHead = CacheFATSize;
        
        if (FileRead(_CacheFile, FAT.begin(), CacheFATSize) != CacheFATSize)
            return;

        for (int i = 0; i < FAT.size(); ++i)
        {
            FileEntry& f = FAT[i];
#ifdef WIN32
            f.Magic = _byteswap_ulong(f.Magic);
            f.Offset = _byteswap_ulong(f.Offset);
            f.Size = _byteswap_ulong(f.Size);
            f.Sequence = _byteswap_ulong(f.Sequence);
#endif
            f.Lock = 0;
        }

        u32 ofs = 0;
        RemoveFilesThatOverlap(ofs, 0);

        for (int i = 0; i < FAT.size(); ++i)
        {
            FileEntry& f = FAT[i];
            if (f.Magic != kMagic || f.Sequence <= Sequence) break;
            Sequence = f.Sequence;
            WriteHead = f.Offset + f.Size;
        }
    }

    u32 FindFileByHash(const CHash& key)
    {
        for (int i = 0; i < FAT.size(); ++i)
        {
            const FileEntry& entry = FAT[i];
            if (entry.Magic == kMagic && entry.Hash == key)
                return i;
        }

        return ~0ul;
    }

public:
    CFilePath FullPath;
    CRawVector<FileEntry> FAT;
protected:
    CFilePath BaseDir;
    FileHandle _CacheFile;
    FileHandle fd_read_cache;
    u32 CacheSizeBytes;
    u32 CacheFATSize;
    u32 WriteHead;
    u32 Sequence;
};

CCache* MakeLazyCache(const char* name, const CFilePath& basedir, u32 size, u32 maxfiles)
{
    return new CFart(name, basedir, size, maxfiles);
}