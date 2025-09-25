#pragma once

#include "refcount.h"
#include "ReflectionVisitable.h"
#include "GuidHash.h"
#include "SerialiseEnums.h"
#include "ResourceEnums.h"
#include "SerialisedResource.h"
#include "Serialise.h"
#include <ResourceTypes.h>


enum EResourceFlag {
	FLAG_REF_COUNT_DIRTY = 2,
	FLAG_CONTAINS_EYETOY = 128,
	FLAG_DONTSWIZZLE = 256,
	FLAG_VOLTEX = 512,
	FLAG_NOSRGB_TEX = 1024,
	FLAGS_BUMP_TEX = 2048,
	FLAGS_TEMPORARY = 8192, // 0x2000
	FLAGS_UNSHARED = 16384, // 0x4000
	FLAGS_MAX_MIP_128 = 65536
};

class CResource : public CReflectionVisitable, public CDependencyWalkable {
private:
    static int gResourceMemoryUsed;
protected:
    CResource(EResourceFlag flags);
public:
#ifdef LBP1
    virtual void Unload();
    virtual ReflectReturn LoadFinished(const SRevision& revision);
    virtual void AddDependencies(CStrongResourceArray&);
    virtual u32 GetSizeInMemory() const;
    virtual u32 GetSizeInGfxMemoryPool(u32) const;
    virtual ~CResource();
#else
    virtual void Unload(); // actually 0x2c???
    virtual ReflectReturn LoadFinished(const SRevision& revision);
    virtual void AddDependencies(CStrongResourceArray&);
    virtual u32 GetSizeInMemory() const;
    virtual u32 GetSizeInGfxMemoryPool(u32) const;
    virtual void Dummy5() = 0; // 0x14
    virtual void Dummy6() = 0; // 0x18
    virtual ~CResource() = 0; // 0x1c/0x20
#endif
public:
    static void IncrementCounters(EResourceLoadState ls, CStreamPriority sp);
    static void DecrementCounters(EResourceLoadState ls, CStreamPriority sp);
public:
    ReflectReturn Duplicate(CResource* resource);
    bool BlockUntilLoaded();

    u32 AddRef();
    u32 Release();

    inline u32 GetRefCount() const { return RefCount; }
    inline u32 GetWeakCount() const { return WeakCount; }

    void UpdateGUIDHash(const CGUID& guid, const CHash& hash);
    inline const CGUID& GetGUID() const { return GUID; }
    inline EResourceType GetType() const { return (EResourceType)ResourceType; }
    inline const CHash& GetLoadedHash() const { return LoadedHash; }

    inline void GetLoadDescriptor(CResourceDescriptorBase& desc) const
    {
        desc = CResourceDescriptorBase((EResourceType)ResourceType, GUID, LoadedHash);
    }
    
    bool HasLoadDescriptor(const CResourceDescriptorBase& desc) const;

    // ReflectReturn SaveToCache(CResourceDescriptorBase&, const CSerialiseControlParams&);
    const char* ResourceName() const;

    void SetFlag(u32, bool);
    inline u32 GetFlags() const { return Flags; }
    bool GetFlag(u32) const;
    bool IsShared() const;
    void Unshare();
    
    inline EResourceLoadState GetLoadState() const { return LoadState; }

    inline bool IsLoaded() const
    {
        #ifdef LBP1
        return LoadState == LOAD_STATE_LOADED;
        #else
        return LoadState == LOAD_STATE_LOADED || LoadState == LOAD_STATE_LOADED_SERIALISING;
        #endif
    }

    inline bool IsLoading() const
    {
        return LoadState == LOAD_STATE_LOADING_DATA;
    }

    inline bool IsPendingDeserialise() const
    {
        return LoadState == LOAD_STATE_PENDING_DESERIALISE;
    }

    inline bool IsError() const
    {
        return LoadState >= LOAD_STATE_ERROR;
    }

    inline CStreamPriority GetPriority() const { return (CStreamPriority)Priority; }

    bool IsBlocking() const;
    void SetPriority(CStreamPriority priority);
    void SetLoadState(EResourceLoadState state);
#ifdef LBP1
public:
    volatile u32 RefCount;
    volatile u32 WeakCount;
    EResourceLoadState LoadState;
    CStreamPriority Priority;
    u32 Flags;
    u32 LazyGCTime;
    CP<CSerialisedResource> CSR;
    EResourceType ResourceType;
    CGUID GUID;
    CHash LoadedHash;
    u32 CachedSizeInMemory;
#else
public:
    CHash LoadedHash;
    CGUID GUID;
    CP<CSerialisedResource> CSR;
    volatile u32 RefCount; // actually a short? 0x28, or bit packed i guess
    volatile u32 WeakCount;
    u32 Flags; // 0x30
    u32 CachedSizeInMemory; // 0x34
    u32 Pad; // 0x38
    struct
    {
        EResourceType ResourceType : 8; // 0x3c
        EResourceLoadState LoadState : 8; // 0x3d
        EStreamPriority Priority : 8; // 0x3e
    };
#endif
};
