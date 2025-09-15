#pragma once

#include <refcount.h>
#include <vector.h>
#include <fifo.h>
#include <CritSec.h>
#include <thread.h>

#include <Resource.h>
#include <ResourceDescriptor.h>
#include <SerialisedResource.h>

#include <FileList.h>

typedef CRawVector<CResource*> CWeakResourceArray;

typedef CPriorityQueue<CP<CSerialisedResource> > CSRQueue;

extern CCriticalSec* gResourceCS;
extern CWeakResourceArray gResourceArray;
extern CVector<CResourceDescriptorBase> gBrokenResources;

extern CSRQueue* CSRsForStaging;
extern CSRQueue* CSRsForSlow;
extern CSRQueue* CSRsForRNP;
extern CSRQueue* CSRsForHTTP;
extern CSRQueue* CSRsDone;
extern CSRQueue* CSRsDoneThreaded;
extern CSRQueue* CSRsFinished;

struct SResourceHeader {
    bool IsText;
    bool IsEncrypted;
    bool Skip4;
    bool HasRevision;

    inline SResourceHeader() : IsText(), IsEncrypted(), Skip4(), HasRevision()
    {

    }

    inline void Reset()
    {
        IsText = false;
        IsEncrypted = false;
        Skip4 = false;
        HasRevision = false;
    }
};

bool CheckTypeBy4ByteHeader(const void* header, SResourceHeader& resource_header, EResourceType expected_type);
const char* Get4ByteHeaderFromType(EResourceType type, SResourceHeader& resource_header);
ESerialisationType GetPreferredSerialisationType(EResourceType type);

struct ResourceLoadSettings {
    CStreamPriority StreamPriority;
    int LoadFlags;
    int ResourceFlags;
    bool CanCreate;
};

class CSerialiseControlParams {
public:
    inline CSerialiseControlParams() : Mode(), BinaryOrTextType(), CompressionLevel(0), CompressionFlags(0), JustGetDeps(), ThreadSafe() {}
public:
    ESerialisationMode Mode;
    ESerialisationType BinaryOrTextType;
    u32 CompressionLevel;
    u8 CompressionFlags;
    bool JustGetDeps;
    bool ThreadSafe;
};

#ifndef LBP1

extern ResourceLoadSettings gLoadSettings;
CP<CResource> LoadResource(const CResourceDescriptorBase& desc, const ResourceLoadSettings& settings);

template<typename T>
CP<T> LoadResourceByKey(int key)
{
    CResourceDescriptor<T> desc(key);
    CP<CResource> resource = LoadResource(desc, gLoadSettings);
    return CP<T>((T*)resource.GetRef());
}

template<typename T>
CP<T> LoadResource(const CResourceDescriptor<T>& desc)
{
    CP<CResource> resource = LoadResource(desc, gLoadSettings);
    return CP<T>((T*)resource.GetRef());
}

#else

CResource* LoadResource(const CResourceDescriptorBase& desc, CStreamPriority priority, unsigned int flags, bool can_create);

template<typename T>
CP<T> LoadResourceByKey(int key, int flags, CStreamPriority prio)
{
    CResourceDescriptor<T> desc(key);
    CP<CResource> resource = LoadResource(desc, flags, prio, false);
    return CP<T>((T*)resource.GetRef());
}

template<typename T>
CP<T> LoadResourceByKey(int key)
{
    CResourceDescriptor<T> desc(key);
    CP<CResource> resource = LoadResource(desc, 0, STREAM_PRIORITY_DEFAULT, false);
    return CP<T>((T*)resource.GetRef());
}

#endif // LBP1

CResource* AllocateNewResource(EResourceType type, u32 flags);
template <typename T>
T* AllocateNewResource(u32 flags)
{
    return (T*) AllocateNewResource(GetResourceType<T>(), flags);
}

#ifndef WIN32

Ib_DeclarePort(BlockUntilResourcesLoaded, bool, CResource** resources, u32 count);
Ib_DeclarePort(UnloadResource, void, CP<CResource> rv);
Ib_DeclarePort(UnloadUnusedResources, void, bool);
Ib_DeclarePort(SerialiseResourceToMemory, ReflectReturn, ByteArray& out, CResource* resource, CSerialiseControlParams const& params, CHash* out_hash);

#else
    bool BlockUntilResourcesLoaded(CResource** resources, u32 count);
    void UnloadResource(CP<CResource> rv);
    void UnloadUnusedResources(bool);
    ReflectReturn SerialiseResourceToMemory(ByteArray& out, CResource* resource, const CSerialiseControlParams& params, CHash* out_hash);
#endif // WIN32

EStreamPriority GetStreamPriority(EResourceType type);
bool IsBrokenResource(const CResourceDescriptorBase& desc);

void RemoveFromResourceList(const CResource* res);
void AddToResourceList(CResource* res);
const char* ResourceTypeString(EResourceType type);

bool InitResourceSystem();
void CloseResourceSystem();
void SetLazyGCSpeed(u32 value);
extern u32 gLazyGCSpeed;
extern THREADID gPumpThreadID;
extern u64 ResourceSystemPumpThreadLastTimeUpdated;
extern bool gResourceSystemInited;