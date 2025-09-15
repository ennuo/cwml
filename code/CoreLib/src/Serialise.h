#pragma once

#include <refcount.h>
#include <vector.h>
#include <fifo.h>
#include <MMString.h>
#include <SerialiseEnums.h>
#include <SerialiseRevision.h>
#include <ReflectionVisitable.h>
#include <GuidHash.h>
#include <BitUtils.h>
#include <utility>

class SRevision {
public:
    inline SRevision(u32 revision, u32 branch_description) :
    Revision(revision), BranchDescription(branch_description)
    {}

    inline u32 GetRevision() const { return Revision; }
    inline u32 GetBranchID() const { return BranchDescription >> 16; }
    inline u32 GetBranchRevision() const { return BranchDescription & 0xffff; }

    inline bool IsAfterRevision(const SRevision& rhs) const
    {
        if (Revision >= rhs.Revision) return true;
        if (GetBranchID() == 0 || GetBranchID() != rhs.GetBranchID())
            return false;
        return GetBranchRevision() >= rhs.GetBranchRevision();
    }

    ReflectReturn CheckRevision() const;
    ReflectReturn CheckBranchDescription() const;
public:
    u32 Revision;
    u32 BranchDescription;
};

// These should be externs, but the compiler is smart enough so who
// gives a shit.
const SRevision gHeadRevision(gFormatRevision, gFormatBranchDescription);
const SRevision gSelfDescribingDependencyRevision(0x109, 0x0);
const SRevision gMinSafeRevision(0x132, 0x0);
const SRevision gCompressedResourcesRevision(0x132, 0x0);
const SRevision gCompressedResources2Revision(0x189, 0x0);
const SRevision gBranchDescriptionFormatRevision(0x271, 0x0);
const SRevision gCompressionFlagsRevision(0x297, 0x4c440002);
extern u32 gMinRevision;

class CReflectionBase;
class CompressionJob;
class DecompressionJob;

class CDependencyCollector { // 82
public:
    virtual ~CDependencyCollector();
    virtual void AddDependency(CReflectionBase*, CDependencyWalkable*, int, const CHash&, const CGUID&);
    virtual bool ToggleDependencies(bool);
};

class PWorld;
class CReflectionBase { // 91
public:
    CReflectionBase();
    virtual ~CReflectionBase();
    // bool GetLimitThingRecursion
    inline bool GetThingPtrAsUID() { return ThingPtrAsUID; }
    // u32 MakeUID
    // void TakeReflectionCS
    inline bool IsGatherVariables() const { return false; }
    inline bool GetCompressInts() const { return false; }
    inline bool GetCompressVectors() const { return false; }
    inline bool GetCompressMatrices() const { return false; }
    // bool CanVisitThing (CThing*)
    inline CStreamPriority* GetLazyCPPriorityPtr() { return &LazyCPPriority; }
    inline CStreamPriority GetLazyCPPriority() { return LazyCPPriority; }
    inline void SetLazyCPPriority(CStreamPriority p) { LazyCPPriority = p; }
    inline bool RequestToAllocate(u64) { return false; }
    // bool GetReflectFast
    // bool AllowNullEntries
    // void SetDependencyCollector(CDependencyCollector*)
    // CDependencyCollector* GetDependencyCollector
    // void AddDependency(CDependencyWalkable*, int, const CHash&, const CGUID&)
    // bool ToggleDependencies(bool)
private:
    u32 NumVisited;
public:
    CRawVector<void*> CanVisitThings;
    PWorld* World;
protected:
    CStreamPriority LazyCPPriority;
public:
    bool ThingPtrAsUID;
    bool TakenReflectionCS;
    MMString<char> TempString;
private:
    CDependencyCollector* DependencyCollector;
};

class CReflectionVisitSave { // 155
    void* GetVisited(CReflectionVisitable*);
    void SetVisited(CReflectionVisitable*, void*);
};

class CReflectionVisitLoad { // 163
public:
    CReflectionVisitLoad();
public:
    void* GetVisited(void*);
    void SetVisited(void*, void*);
private:
    CRawVector<void*> Visited;
};

class CReflectionLoadVector : public CReflectionBase, public CReflectionVisitLoad { // 181
public:
    CMMSemaphore& GetSem();
    CReflectionLoadVector(const CBaseVector<char>*);
    virtual ~CReflectionLoadVector();
    inline bool GetSaving() { return false; }
    inline bool GetLoading() { return true; }
    inline u32 GetRevision() { return Revision.GetRevision(); }
    inline u32 GetBranchDescription() { return Revision.BranchDescription; }
    inline u16 GetBranchID() { return Revision.GetBranchID(); }
    inline u16 GetBranchRevision() { return Revision.GetBranchRevision(); }
    inline void SetRevision(const SRevision& revision) { Revision = revision; }
    ReflectReturn ReadWrite(void* d, int size);
    
    inline bool RequestToAllocate(u64 size)
    {
        Allocated += size;
        return Allocated < 9999999; // ~10mbs
    }

    inline u8 GetCompressionFlags() const { return CompressionFlags; }
    inline void SetCompressionFlags(u8 flags) { CompressionFlags = flags; }
    inline bool GetCompressInts() const { return CompressionFlags & COMPRESS_INTS; }
    inline bool GetCompressVectors() const { return CompressionFlags & COMPRESS_VECTORS; }
    inline bool GetCompressMatrices() const { return CompressionFlags & COMPRESS_MATRICES; }
    ReflectReturn LoadCompressionData(u32*);
    ReflectReturn CleanupDecompression();
    ReflectReturn PumpDecompression();
    ReflectReturn WaitForHead();
    ReflectReturn Decrypt();
    void SetVector(const CBaseVector<char>*);
    u32 GetVecLeft();
public:
    CReflectionLoadVector(const CReflectionLoadVector&);
    CReflectionLoadVector& operator=(const CReflectionLoadVector&);
protected:
    const CBaseVector<char>* Vec;
    u64 Allocated;
    CRawVector<char> DecryptedVec;
public:
    SRevision Revision;
    u32 LoadPos;
    u32 OutstandingBuffers;
    CP<DecompressionJob> CurJob;
    CP<DecompressionJob> PendingInit;
private:
    CMMSemaphore* Sem;
public:
    u32 JobsTag;
    bool SPUDecompressAvailable;
private:
    u8 CompressionFlags;
};

class CReflectionSaveVector : public CReflectionBase, CReflectionVisitSave { // 259
public:
    CReflectionSaveVector(ByteArray*, u32);
    virtual ~CReflectionSaveVector();
public:
    inline bool GetSaving() { return true; }
    inline bool GetLoading() { return false; }
    inline u32 GetRevision();
    u32 GetBranchDescription();
    u16 GetBranchID();
    u16 GetBranchRevision();
    void SetRevision(const SRevision& revision);
    inline u8 GetCompressionFlags() const { return CompressionFlags; }
    inline void SetCompressionFlags(u8 flags) { CompressionFlags = flags; }
    inline bool GetCompressInts() const { return CompressionFlags & COMPRESS_INTS; }
    inline bool GetCompressVectors() const { return CompressionFlags & COMPRESS_VECTORS; }
    inline bool GetCompressMatrices() const { return CompressionFlags & COMPRESS_MATRICES; }
    ReflectReturn ReadWrite(void*, int);
    ReflectReturn StartCompressing();
    ReflectReturn FinishCompressing();
    ReflectReturn PumpCompression();
    void StartEncryptedBlock();
    ReflectReturn FinishEncryptedBlock();
    bool GetSaveCompressionEnabled() const;
    void SetVector(ByteArray*);
private:
    CReflectionSaveVector(const CReflectionSaveVector&);
    CReflectionSaveVector& operator=(const CReflectionSaveVector&);
private:
    ByteArray* Vec;
    CP<CompressionJob> CurJob;
    CP<CompressionJob> PendingCleanup;
    CMMSemaphore Sem;
private:
    u32 EncryptedBlockStart;
public:
    u32 JobsTag;
    u32 CompressionLevel;
    bool SPUCompressAvailable;
private:
    u8 CompressionFlags;
};

template <typename T> inline bool IsVectorCompressible() { return false; }
template <> inline bool IsVectorCompressible<CRawVector<u32> >() { return true; }
template <> inline bool IsVectorCompressible<CRawVector<s32> >() { return true; }
template <> inline bool IsVectorCompressible<CRawVector<u64> >() { return true; }
template <> inline bool IsVectorCompressible<CRawVector<s64> >() { return true; }

template <typename R, typename D>
ReflectReturn ReflectCompressedInt(R& r, D& d)
{
    ReflectReturn ret = REFLECT_OK;

    if (r.GetLoading())
    {
        d = 0;
        D shift = 0;
        do 
        {
            u8 b;
            ret = r.ReadWrite((void*)&b, sizeof(u8));
            d |= (b & 0x7f) << (shift & 0x3f);
            if ((b & 0x80) == 0) return ret;
            shift += 7;
        } 
        while (ret == REFLECT_OK);
    }

    if (r.GetSaving())
    {
        D value = d;
        do
        {
            u8 b = value & 0x7f;
            value >>= 7;
            if (value != 0) b |= 0x80;
            ret = r.ReadWrite((void*)&b, sizeof(u8));
            if (value == 0) return ret;
        }
        while (ret == REFLECT_OK);
    }
    
    return ret;
}

typedef CReflectionLoadVector CLoadVector;
typedef CReflectionSaveVector CSaveVector;

#define READ_WRITE_FAST(T) \
template <typename R> \
ReflectReturn Reflect(R& r, T& d) \
{ \
    return r.ReadWrite((void*)&d, sizeof(T)); \
}

#ifdef PS3
    #define READ_WRITE_16(T) READ_WRITE_FAST(T)
    #define READ_WRITE_32(T) READ_WRITE_FAST(T)
    #define READ_WRITE_64(T) READ_WRITE_FAST(T)
#else

    #define READ_WRITE_16(T) \
    template <typename R> \
    ReflectReturn Reflect(R& r, T& d) \
    { \
        u16 swap = _byteswap_ushort(*((u16*)&d)); \
        ReflectReturn ret = r.ReadWrite((void*)&swap, sizeof(u16)); \
        *((u16*)&d) = _byteswap_ushort(swap); \
        return ret; \
    } \

    #define READ_WRITE_32(T) \
    template <typename R> \
    ReflectReturn Reflect(R& r, T& d) \
    { \
        u32 swap = _byteswap_ulong(*((u32*)&d)); \
        ReflectReturn ret = r.ReadWrite((void*)&swap, sizeof(u32)); \
        *((u32*)&d) = _byteswap_ulong(swap); \
        return ret; \
    } \

    #define READ_WRITE_64(T) \
    template <typename R> \
    ReflectReturn Reflect(R& r, T& d) \
    { \
        u64 swap = _byteswap_uint64(*((u64*)&d)); \
        ReflectReturn ret = r.ReadWrite((void*)&swap, sizeof(u64)); \
        *((u64*)&d) = _byteswap_uint64(swap); \
        return ret; \
    }

#endif

READ_WRITE_FAST(CHash);

READ_WRITE_FAST(char);
READ_WRITE_16(wchar_t);

READ_WRITE_FAST(s8);
READ_WRITE_FAST(u8);
READ_WRITE_16(s16);
READ_WRITE_16(u16);

READ_WRITE_64(s64);

template <typename R>
ReflectReturn Reflect(R& r, u32& h)
{
    if (r.GetCompressInts()) return ReflectCompressedInt(r, h);
    else 
    {
#ifdef PS3
        return r.ReadWrite((void*)&h, sizeof(u32));
#else
        u32 swap = _byteswap_ulong(h);
        ReflectReturn ret = r.ReadWrite((void*)&swap, sizeof(u32));
        h = _byteswap_ulong(swap);
        return ret;
#endif
    }
}

template <typename R>
ReflectReturn Reflect(R& r, s32& h)
{
    if (r.GetCompressInts()) 
    {
        h = ZigZagInt(h);
        ReflectReturn res = ReflectCompressedInt(r, h);
        h = UnZigZagInt(h);
        return res;
    }
    else 
    {
#ifdef PS3
        return r.ReadWrite((void*)&h, sizeof(s32));
#else
        s32 swap = _byteswap_ulong(h);
        ReflectReturn res = r.ReadWrite((void*)&swap, sizeof(s32));
        h = _byteswap_ulong(swap);
        return res;
#endif
    }
}

template <typename R>
ReflectReturn Reflect(R& r, u64& h)
{
    if (r.GetCompressInts()) return ReflectCompressedInt(r, h);
    else 
    {
#ifdef PS3
        return r.ReadWrite((void*)&h, sizeof(u32));
#else
        u64 swap = _byteswap_uint64(h);
        ReflectReturn ret = r.ReadWrite((void*)&swap, sizeof(u64));
        h = _byteswap_uint64(swap);
        return ret;
#endif
    }
}

READ_WRITE_32(float);
READ_WRITE_FAST(bool);

#ifdef PS3
    READ_WRITE_FAST(v4);
#else
    template <typename R>
    ReflectReturn Reflect(R& r, v4& v)
    {
        return REFLECT_NOT_IMPLEMENTED;
    }
#endif

template <typename R>
ReflectReturn Reflect(R& r, CGUID& d)
{
    return Reflect(r, d.guid);
}

template <typename R>
ReflectReturn Reflect(R& r, MMString<char>& d)
{
    ReflectReturn res;
    s32 size = d.size();
    res = Reflect(r, size);
    if (res != REFLECT_OK) return res;

    // game technically calls begin which calls operator[0], but its the same as c_str
    d.resize(size, '\0');
    return r.ReadWrite((void*)d.begin(), size * sizeof(char));
}

template <typename R, typename D>
ReflectReturn Add(R& r, D& d, const char* c);

template<typename R, typename D>
ReflectReturn CompressedReflectVectorContents(R& r, D& d, u32 size)
{
    const u32 elemsz = sizeof(*d.begin());
    if (r.GetLoading()) memset(d.begin(), 0, elemsz * size);

    ReflectReturn rv;
    
    u8 bytes = 0;
    if (r.GetSaving()) bytes = elemsz; // for now just disable it when saving

    if ((rv = Reflect(r, bytes)) != REFLECT_OK) return rv;

    u8* data = (u8*)d.begin();
    for (u32 i = 0; i < bytes; ++i)
    for (u32 j = 0; j < size; ++j)
    {
#ifdef WIN32
        u8& byte = data[(j * elemsz) + i];
#else
        u8& byte = data[(j * elemsz) + (elemsz - 1 - i)];
#endif
        if ((rv = Reflect(r, byte)) != REFLECT_OK)
            return rv;
    }

    return REFLECT_OK;
}

template<typename R, typename D>
ReflectReturn GeneralReflectVectorContents(R& r, D& d, u32 size)
{
    for (u32 i = 0; i < size; ++i)
    {
        ReflectReturn ret = Add(r, d[i], NULL);
        if (ret != REFLECT_OK)
            return ret;
    }

    return REFLECT_OK;
}

template<typename R, typename D>
ReflectReturn ReflectVectorContents(R& r, D& d, u32 size)
{
    if (!r.IsGatherVariables() && IsVectorCompressible<D>() && r.GetCompressVectors())
        return CompressedReflectVectorContents(r, d, size);
    return GeneralReflectVectorContents(r, d, size);
}

template<typename R, typename D>
ReflectReturn ReflectVector(R& r, D& d)
{
    ReflectReturn ret;

    u32 size = d.size();
    if (!r.IsGatherVariables())
    {
        if ((ret = Reflect(r, size)) != REFLECT_OK) 
            return ret;       
        if (!r.RequestToAllocate(size * sizeof(D)))
            return REFLECT_EXCESSIVE_ALLOCATIONS;
    }

    if (r.GetLoading())
    {
        if (d.begin() == NULL && d.size() != 0)
        {
            d.GetSizeForSerialisation() = 0;
            d.resize(0);
        }

        d.clear();
        d.resize(size);
    }

    return ReflectVectorContents(r, d, size);
}

template<typename R, typename D>
ReflectReturn Reflect(R& r, CVector<D>& d)
{
    return ReflectVector<R, CVector<D> >(r, d);
}

template<typename R, typename D>
ReflectReturn Reflect(R& r, CRawVector<D>& d)
{
    return ReflectVector<R, CRawVector<D> >(r, d);
}

template<typename R>
ReflectReturn Reflect(R& r, ByteArray& d)
{
    return ReflectVector<R, ByteArray>(r, d);
}

template <typename R, typename T1, typename T2>
ReflectReturn Reflect(R& r, std::pair<T1, T2>& d)
{
    ReflectReturn ret;
    if ((ret = Reflect(r, d.first)) != REFLECT_OK) return ret;
    return Reflect(r, d.second);
}

template <typename R, typename D>
ReflectReturn Add(R& r, D& d, const char* c) // 955
{
    return Reflect(r, d);
}

#undef READ_WRITE_FAST
#undef READ_WRITE_16
#undef READ_WRITE_32
#undef READ_WRITE_64
