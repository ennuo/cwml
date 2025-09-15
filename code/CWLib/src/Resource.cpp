#include "Resource.h"
#include "ResourceSystem.h"
#include "thread.h"
#include <GuidHashMap.h>
#include <DebugLog.h>

#ifdef WIN32
    #include <windows.h>
#else
    #include <cell/atomic.h>
#endif


int CResource::gResourceMemoryUsed;

void CResource::IncrementCounters(EResourceLoadState ls, CStreamPriority sp)
{

}

void CResource::DecrementCounters(EResourceLoadState ls, CStreamPriority sp)
{

}

CResource::CResource(EResourceFlag flags) : CReflectionVisitable(), CDependencyWalkable(),
RefCount(0), Priority(STREAM_PRIORITY_LOW), CSR(), WeakCount(0), GUID(0), LoadedHash(),
CachedSizeInMemory(0)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    Flags = flags | FLAG_REF_COUNT_DIRTY;
    LoadState = LOAD_STATE_LOADED;
    Priority = STREAM_PRIORITY_LOW;
    // LazyGCTime = gLazyGCTime;
    IncrementCounters(LOAD_STATE_LOADED, STREAM_PRIORITY_LOW);
    
    if ((Flags & FLAGS_TEMPORARY) == 0)
        AddToResourceList(this);
    else
        Flags |= FLAGS_UNSHARED;
        
    if ((Flags & FLAGS_UNSHARED) == 0)
        AddRef();
}

CResource::~CResource()
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    if ((Flags & FLAGS_TEMPORARY) == 0)
        RemoveFromResourceList(this);
    Unload();
    DecrementCounters(LoadState, Priority);
}

void CResource::Unload()
{
    if (LoadState != LOAD_STATE_UNLOADED)
    {
        MMLogCh(DC_RESOURCE, "Unloading resource, type %s, guid %d, hash #%s from thread %lld (AmInMainThread %d)\n",
            ResourceTypeString((EResourceType)ResourceType),
            GUID.guid,
            StringifyHash(LoadedHash).c_str(),
            (u64)GetCurrentThreadId(),
            (int)AmInMainThread()
        );
    }

    if (CachedSizeInMemory != 0)
        CachedSizeInMemory = MAX(gResourceMemoryUsed - CachedSizeInMemory, 0);

    CSR = NULL;
    
    SetLoadState(LOAD_STATE_UNLOADED);
    
}

void CResource::AddDependencies(CStrongResourceArray&)
{

}

u32 CResource::GetSizeInGfxMemoryPool(u32) const
{
    return 0;
}

u32 CResource::GetSizeInMemory() const
{
    return 0;
}

ReflectReturn CResource::LoadFinished(const SRevision&)
{
    return REFLECT_OK;
}

void CResource::SetLoadState(EResourceLoadState state)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    if (LoadState == state) return;
    IncrementCounters(state, Priority);
    DecrementCounters(LoadState, Priority);
    LoadState = state;
}

void CResource::SetPriority(CStreamPriority priority)
{
#ifdef LBP1 // todo: fix me!!
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    if (Priority.GetRawBits() == priority.GetRawBits()) return;
    IncrementCounters(LoadState, priority);
    DecrementCounters(LoadState, Priority);
    Priority = priority;
#endif
}

u32 CResource::AddRef() 
{
    Flags |= FLAG_REF_COUNT_DIRTY;
#ifdef WIN32
    return InterlockedIncrement((LONG*)&RefCount) - 1;
#else
    return cellAtomicIncr32((uint32_t*) &this->RefCount);
#endif
}

u32 CResource::Release() 
{
    Flags |= FLAG_REF_COUNT_DIRTY;
#ifdef WIN32
    return InterlockedDecrement((LONG*)&RefCount) + 1;
#else
    return cellAtomicDecr32((uint32_t*) &this->RefCount);
#endif
}

bool CResource::BlockUntilLoaded()
{
#ifdef WIN32
    CResource* resource = this;
    return BlockUntilResourcesLoaded(&resource, 1);
#endif
    return false;
}

void CResource::UpdateGUIDHash(const CGUID& guid, const CHash& hash)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    if ((Flags & FLAGS_TEMPORARY) == 0)
        RemoveFromResourceList(this);

    GUID = guid;
    LoadedHash = hash;
    
    if ((Flags & FLAGS_TEMPORARY) == 0)
        AddToResourceList(this);
}

const char* CResource::ResourceName() const
{
    static char buffer[CHash::kHashHexStringSize + 1];
    if (GUID)
    {
        const CFileDBRow* row = FileDB::FindByGUID(GUID);
        if (row != NULL) return row->GetFilename();
        sprintf(buffer, GUID < 0 ? "g0x%08x" : "g%d", GUID.guid);
        return buffer;
    }

    buffer[0] = 'h';
    LoadedHash.ConvertToHex(reinterpret_cast<char(&)[CHash::kHashHexStringSize]>(buffer[1]));
    return buffer;
}