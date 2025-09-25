#include <ResourceSystem.h>

#include <DebugLog.h>
#include <algorithm>
#include <set>

#include <GuidHashMap.h>
#include <System.h>
#include <Fart.h>
#include <FartRO.h>
#include <Clock.h>
#include <Variable.h>
#include <thread.h>
#include <GameUpdateStage.h>
#include <SharedSerialise.h>

#include <ResourceGuidSubst.h>
#include <ResourceDLC.h>
#include <ResourceScript.h>



#ifdef RESOURCE_SYSTEM_REIMPLEMENTATION

CCriticalSec* gResourceCS;
CWeakResourceArray gResourceArray;
CVector<CResourceDescriptorBase> gBrokenResources;

CSRQueue* CSRsForStaging;
CSRQueue* CSRsForSlow;
CSRQueue* CSRsForRNP;
CSRQueue* CSRsForHTTP;
CSRQueue* CSRsDone;
CSRQueue* CSRsDoneThreaded;
CSRQueue* CSRsFinished;

THREADID gPumpThreadID;
u64 ResourceSystemPumpThreadLastTimeUpdated;
u32 gLazyGCSpeed;

bool gResourceSystemInited;


struct SCompareResource {
    bool operator()(const CResource* lhs, const CResource* rhs) const
    {
        if (lhs->GetGUID() == rhs->GetGUID())
            return lhs->GetLoadedHash() < rhs->GetLoadedHash();
        return lhs->GetGUID() < rhs->GetGUID();
    }

    bool operator()(const CResource* lhs, const CResourceDescriptorBase& rhs) const
    {
        if (lhs->GetGUID() == rhs.GetGUID())
            return lhs->GetLoadedHash() < rhs.GetHash();
        return lhs->GetGUID() < rhs.GetGUID();
    }

    bool operator()(const CResourceDescriptorBase& lhs, const CResource* rhs) const
    {
        if (lhs.GetGUID() == rhs->GetGUID())
            return lhs.GetHash() < rhs->GetLoadedHash();
        return lhs.GetGUID() < rhs->GetGUID();
    }
};

bool CompareResourceDescriptor(const CResource* res, const CResourceDescriptorBase& desc)
{
    if (res == NULL || res->GetType() != desc.GetType())
        return false;

    if (!res->GetGUID())
        return !desc.GetGUID() || res->LoadedHash == desc.GetHash();
    else
        return res->GetGUID() == desc.GetGUID();
}

CResource* AllocateNewResource(EResourceType type, u32 flags)
{
    CResource* res;
    switch (type)
    {
#define RESOURCE_MACRO(type, class_name, headers) case type: res = new class_name((EResourceFlag)flags); break;
    #include <ResourceList.h>
#undef RESOURCE_MACRO
        default:
            return NULL;
    }

    if (res != NULL)
        res->ResourceType = type;

    return res;
}

CResource* FindResourceInList(const CResourceDescriptorBase& csr)
{
    if (!csr.IsValid()) return NULL;

    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    CResource** it = std::lower_bound(gResourceArray.begin(), gResourceArray.end(), csr, SCompareResource());

    CResource* res = NULL;
    while (it != gResourceArray.end())
    {
        res = *it;

        if (!CompareResourceDescriptor(res, csr))
            return NULL;
    
        if ((res->GetFlags() & FLAGS_UNSHARED) == 0)
            break;
    }

    return res;
}

CResource* FindResourceInList(const CSerialisedResource* csr)
{
    if (csr == NULL) return NULL;
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    CResource* res = FindResourceInList(csr->GetDescriptor());
    if (res == NULL || csr != res->CSR)
    {
        for (int i = 0; i < gResourceArray.size(); ++i)
        {
            res = gResourceArray[i];
            if (res->CSR == csr)
                return res;
        }
    }

    return res;
}

const char* GetResourceErrorDescription(EResourceLoadState err)
{
    return "";
}

EStreamPriority GetStreamPriority(EResourceType type)
{
    switch (type)
    {
        case RTYPE_MESH:
        case RTYPE_GFXMATERIAL:
        case RTYPE_TEXTURE_LIST:
            return STREAM_PRIORITY_LOW;
        case RTYPE_TEXTURE: 
            return STREAM_PRIORITY_MED;
        default:
            return STREAM_NO_STREAMING;
    }
}

ESerialisationType GetPreferredSerialisationType(EResourceType type) // 354
{
    switch (type)
    {
        case RTYPE_FILENAME:
        case RTYPE_FONTFACE:
            return PREFER_FILE;
        
        case RTYPE_GUID_SUBST:
        case RTYPE_SETTINGS_CHARACTER:
        case RTYPE_SETTINGS_SOFT_PHYS:
        case RTYPE_EDITOR_SETTINGS:
        case RTYPE_JOINT:
        case RTYPE_GAME_CONSTANTS:
        case RTYPE_POPPET_SETTINGS:
        case RTYPE_SETTINGS_NETWORK:
        case RTYPE_PARTICLE_SETTINGS:
        // case RTYPE_PARTICLE_TEMPLATE:
        // case RTYPE_PARTICLE_LIBRARY:
        case RTYPE_AUDIO_MATERIALS:
        case RTYPE_SETTINGS_FLUID:
        case RTYPE_TEXTURE_LIST:
        case RTYPE_MUSIC_SETTINGS:
        case RTYPE_MIXER_SETTINGS:
            return PREFER_TEXT;
        
        default: 
            return PREFER_BINARY;
    }
}

void SetResourceError(CResource* res, EResourceLoadState error)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    if (res == NULL)
    {
        MMLogCh(DC_RESOURCE, "!!!!!! RESOURCE ERROR (errmsg %s, error %d)\n", GetResourceErrorDescription(error), error);
        return;
    }

    CResourceDescriptorBase desc;
    res->GetLoadDescriptor(desc);

    if (!desc.GetGUID())
    {

    }

    MMLog("%d\n", error);

    res->SetLoadState(error);
    MMLogCh(DC_RESOURCE, "!!!!!! RESOURCE ERROR %s %s\n", res->ResourceName(), GetResourceErrorDescription(error));
    res->CSR = NULL;
}

void SetResourceError(const CSerialisedResource* res, EResourceLoadState error)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    CResource* resource = FindResourceInList(res);
    if (resource == NULL)
    {
        MMLogCh(DC_RESOURCE, "?? serialized resource unable to set error, missing resource\n");
        return;
    }

    SetResourceError(resource, error);
}

void RemoveFromResourceList(const CResource* res)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    CResource** it = std::lower_bound(gResourceArray.begin(), gResourceArray.end(), res, SCompareResource());
    if (it != gResourceArray.end() && *it == res)
        gResourceArray.erase(it);
}

void AddToResourceList(CResource* res)
{
    if (res == NULL || (res->GetFlags() & FLAGS_TEMPORARY) != 0) return;
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    CResource** it = std::lower_bound(gResourceArray.begin(), gResourceArray.end(), res, SCompareResource());
    gResourceArray.insert(it, res);
}

void AddCSRToQueue(const CP<CSerialisedResource>& csr, CSRQueue* queue, CStreamPriority priority)
{
    if (!csr) return;
    queue->Push(priority.GetPriority(), csr);
}

void AddCSRToDoneQueue(const CP<CSerialisedResource>& csr, CStreamPriority prio)
{
    AddCSRToQueue(csr, CSRsDone, prio);
}

void LoadResource(CResource* res, CStreamPriority priority)
{
    if (res == NULL) return;
    CCSLock lock(gResourceCS, __FILE__, __LINE__);

    if (res->IsLoaded() || res->IsError() || priority.FlagIsSet(STREAM_PRIORITY_DONT_LOAD)) return;

    if (priority.FlagIsSet(STREAM_PRIORITY_DEFAULT))
        priority.SetPriority(GetStreamPriority(res->GetType()));

    if (res->LoadState == LOAD_STATE_UNLOADED)
    {
        MMLogCh(DC_RESOURCE, "Loading resource g%d #%s\n", res->GetGUID().guid, StringifyHash(res->GetLoadedHash()).c_str());
        if (WantQuitOrWantQuitRequested())
        {
            SetResourceError(res, (EResourceLoadState)REFLECT_APPLICATION_QUITTING);
            return;
        }

        res->SetPriority(priority);

        res->CSR = new CSerialisedResource(CResourceDescriptorBase(
            res->GetType(),
            res->GetGUID(),
            res->GetLoadedHash()
        ));

        res->SetLoadState(LOAD_STATE_LOADING_DATA);
        AddCSRToQueue(res->CSR, CSRsForStaging, res->Priority);
    }
    else if (priority.FlagIsSet(STREAM_PRIORITY_DONT_DESERIALISE) && res->Priority.FlagIsSet(STREAM_PRIORITY_DONT_DESERIALISE) && res->IsPendingDeserialise())
    {
        res->SetPriority(priority);
        AddCSRToDoneQueue(res->CSR, priority);
    }
    else if (res->IsLoading() && priority.GetPriority() < res->Priority.GetPriority())
    {
        res->SetPriority(priority);

        CSRsForStaging->ChangePriority(priority.GetPriority(), res->CSR);
        CSRsForSlow->ChangePriority(priority.GetPriority(), res->CSR);
        CSRsForRNP->ChangePriority(priority.GetPriority(), res->CSR);
        CSRsForHTTP->ChangePriority(priority.GetPriority(), res->CSR);
    }
}

CResource* LoadResource(const CResourceDescriptorBase& desc, CStreamPriority priority, unsigned int flags, bool can_create)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    if (desc.GetType() == RTYPE_LOCAL_PROFILE || desc.GetType() == RTYPE_SYNCED_PROFILE)
        flags |= FLAGS_TEMPORARY;
    
    CGUID remap;
    if (desc.GetGUID() && NGuidSubst::DoGUIDSubstitution(desc.GetGUID(), remap))
        return LoadResource(CResourceDescriptorBase(desc.GetType(), remap), priority, flags, can_create);

    // if (desc.GetGUID() < 0 && FileDB::RemapLocalGUID(desc.GetGUID(), remap))
    // {

    // }


    CResource* res = NULL;
    if ((flags & FLAGS_TEMPORARY) != 0 || (res = FindResourceInList(desc)) == NULL || (res->GetFlags() & FLAGS_TEMPORARY) != 0)
    {
        res = AllocateNewResource(desc.GetType(), flags);
        if (!desc.GetGUID())
        {
            res->UpdateGUIDHash(CGUID::Zero, desc.GetHash());
        }
        else
        {
            const CFileDBRow* row = FileDB::FindByGUID(desc.GetGUID());
            if (row == NULL)
            {
                res->UpdateGUIDHash(desc.GetGUID(), CHash::Zero);
                if (can_create)
                    return res;
            }
            else
            {
                res->UpdateGUIDHash(desc.GetGUID(), row->GetHash());
            }
        }

        res->SetLoadState(LOAD_STATE_UNLOADED);
    }

    LoadResource(res, priority);

    return res;
}

void UnloadResource(CP<CResource> rv)
{
    if (!rv) return;
    
    CCSLock lock(gResourceCS, __FILE__, __LINE__);

    CP<CSerialisedResource> csr = rv->CSR;
    rv->CSR = NULL;

    CSRsForStaging->Erase(csr);
    CSRsForSlow->Erase(csr);
    CSRsForRNP->Erase(csr);
    CSRsForHTTP->Erase(csr);
    CSRsDone->Erase(csr);
    CSRsDoneThreaded->Erase(csr);
    CSRsFinished->Erase(csr);

    rv->Unload();
}

bool IsBrokenResource(const CResourceDescriptorBase& desc)
{
    return false;
}

void GetResourceSet(std::set<CP<CResource> >& resources)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    for (int i = 0; i < gResourceArray.size(); ++i)
        resources.insert(gResourceArray[i]);
}

void UnloadUnusedResources(bool aggressive, u32&, u32, u32)
{
    
}

void UnloadUnusedResources(bool aggressive)
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    CMainGameStageOverride main_game_stage_override(E_UPDATE_STAGE_LOADING);
    // do UnloadUnusedResources(aggressive);
    // while (aggressive);
}

void UnloadAllResources()
{
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    
    // who knows man
    std::set<CP<CResource> > resources;
    GetResourceSet(resources);
    resources.clear();
    GetResourceSet(resources);

    for (std::set<CP<CResource> >::iterator it = resources.begin(); it != resources.end(); ++it)
        UnloadResource(*it);
    UnloadUnusedResources(true);
}

const char* GetResourceIdString(EResourceType type)
{
    switch (type)
    {
#define RESOURCE_MACRO(type, class_name, headers) case type: return headers;
#define RESOURCE_MACRO_UNIMPLEMENTED RESOURCE_MACRO
    #include <ResourceList.h>
#undef RESOURCE_MACRO_UNIMPLEMENTED
#undef RESOURCE_MACRO
        default: return NULL;
    }
}

bool CheckTypeBy4ByteHeader(const void* header, SResourceHeader& resource_header, EResourceType expected_type) // 278
{
    resource_header.Reset();

    const char* headerid = (const char*)header;
    u32 magic = *(u32*)header;

    if (expected_type == RTYPE_LEVEL && magic == 0)
    {
        resource_header.HasRevision = true;
        resource_header.Skip4 = true;
        return true;
    }

    const char* resourceid = GetResourceIdString(expected_type);
    resource_header.HasRevision = resourceid != NULL;
    if (resourceid == NULL) return true;

    const int* headers = (const int*)resourceid;
    int size = StringLength(resourceid) / 4;
    if (size == 0) return false;

    bool valid = false;
    for (int i = 0; i < size; ++i)
    {
        if (headers[i] == magic)
            valid = true;
    }

    if (!valid) return false;

    char type = headerid[3];
    resource_header.Skip4 = true;
    if (type == 't') resource_header.IsText = true;
    if (type == 'e') resource_header.IsEncrypted = true;

    return true;
}

const char* Get4ByteHeaderFromType(EResourceType type, SResourceHeader& resource_header)
{
    const char* id = GetResourceIdString(type);

    int size;
    if (id == NULL || (size = StringLength(id) / 4) == 0)
    {
        resource_header.IsEncrypted = false;
        resource_header.HasRevision = false;
        resource_header.IsText = false;

        return NULL;
    }

    resource_header.HasRevision = true;

    if (resource_header.IsText)
    {
        for (int i = 0; i < size; ++i)
            if (id[i * 4 + 3] == 't')
                return id + (i * 4);
        resource_header.IsText = false;
    }

    if (resource_header.IsEncrypted)
    {
        for (int i = 0; i < size; ++i)
            if (id[i * 4 + 3] == 'e')
                return id + (i * 4);
        resource_header.IsEncrypted = false;
    }

    for (int i = 0; i < size; ++i)
    {
        if (id[i * 4 + 3] == 'b')
            return id + (i * 4);
    }

    return NULL;
}

void DeserialiseResource(CP<CResource>& res) // 1672
{
    if (WantQuit())
    {
        SetResourceError(res, (EResourceLoadState)REFLECT_APPLICATION_QUITTING);
        res->CSR = NULL;
        return;
    }

    CSerialiseControlParams params;
    params.Mode = MODE_LOAD;
    params.BinaryOrTextType = PREFER_DEFAULT;
    params.CompressionFlags = 0x1; // default should be 7
    params.JustGetDeps = false;
    params.CompressionLevel = 0x7;
    params.ThreadSafe = false;

    ReflectReturn rv = SerialiseResource(*res, params, NULL);
    if (rv != REFLECT_OK) SetResourceError(res, (EResourceLoadState)rv);
    res->CSR = NULL;
}

ReflectReturn SerialiseResourceToMemory(ByteArray& out, CResource* resource, const CSerialiseControlParams& params, CHash* out_hash)
{
    if (resource == NULL) return REFLECT_RESOURCE_IN_WRONG_STATE;
    CCSLock lock(gResourceCS, __FILE__, __LINE__);
    if (resource->CSR) return REFLECT_RESOURCE_IN_WRONG_STATE;
    resource->SetPriority(STREAM_NO_STREAMING);

    resource->CSR = new CSerialisedResource(CResourceDescriptorBase(
        resource->GetType(),
        resource->GetGUID(),
        resource->GetLoadedHash()
    ));

    resource->CSR->Data.swap(out);
    ReflectReturn rv = SerialiseResource(*resource, params, out_hash);
    resource->CSR->Data.swap(out);

    resource->CSR = NULL;

    return rv;
}

void ReadDepsData(CP<CResource> res)
{

}

bool ResourceSystemPumpWork(CSRQueue* queue, int timeout)
{
    CP<CSerialisedResource> csr;
    int priority;

    if (!queue->Pop(priority, csr, timeout))
        return false;
    
    CP<CResource> res = FindResourceInList(csr);
    if (res)
    {
        res->SetLoadState(LOAD_STATE_PENDING_DESERIALISE);
        res->CSR = csr;
        if (res->GetType() == RTYPE_FILENAME || !res->GetPriority().FlagIsSet(STREAM_PRIORITY_DONT_DESERIALISE))
            DeserialiseResource(res);
        else
            ReadDepsData(res);
        
        return true;

    }
    else MMLogCh(DC_RESOURCE, "?? serialized resource discarded - not needed\n");

    return false;
}

bool ResourceSystemPump(bool block)
{
    CSRQueue* queue;
    bool main_thread = AmInMainThread();
    if (!main_thread)
    {
        if (GetCurrentThreadId() != gPumpThreadID)
        {
            if (block) ThreadSleep(5);
            return false;
        }

        queue = CSRsDoneThreaded;
    }
    else queue = CSRsDone;

    if (queue)
    {
        u64 start = GetClock();
        do
        {
            if (!ResourceSystemPumpWork(queue, block ? 5 : 0)) break;
            // if (main_thread)
            //     DrawLoadingScreenIfLotsOfTimeHasPassed();
        }
        while (GetClock() - start < 11);
    }

    if (main_thread)
    {
        // DrawLoadingScreenIfLotsOfTimeHasPassed();
        // if (g_lazy_gc_speed != 0)
        //     UnloadUnusedResourcesLazy(gBrokenResources, g_lazy_gc_speed);
    }

    return false;
}

void DrainResourceSystem()
{
    SetLazyGCSpeed(0);
    if (!gResourceSystemInited) return;
    MMLogCh(DC_INIT, "Shutting down so waiting for pending resources to clear the hell out @ %f\n", GetClockSeconds());
    // while (true)
    // {
    //     ResourceSystemPump(false);
    //     break;
    // }

}

bool CheckWaitingResources(CResource** wait_for_resources, u32 count, bool& error)
{
    error = false;
    for (int i = 0; i < count; ++i)
    {
        CResource* res = wait_for_resources[i];
        if (res == NULL) continue;

        if (res->IsError()) error = true;
        if (!res->IsLoaded()) 
            return false;
    }

    return true;
}

bool BlockUntilLoaded_(EWaitForStreamingResources dont_wait_for_streaming_resources, CResource** resources, u32 count)
{
    bool error = false;
    if (resources != NULL && CheckWaitingResources(resources, count, error))
        return !error;

    for (int i = 0; i < count; ++i)
    {
        CResource* res = resources[i];
        while (!WantQuitOrWantQuitRequested())
        {
            if (res == NULL || res->IsError() || res->IsLoaded()) break;
            
            ResourceSystemPump(resources != NULL);
            
            if (!dont_wait_for_streaming_resources)
            {
                // not implemented
            }

            if (resources != NULL && CheckWaitingResources(resources, count, error))
                return !error;
        }
    }

    return !error;
}

bool BlockUntilResourcesLoaded(CResource** resources, u32 count)
{
    if (count == 1 && *resources != NULL && (*resources)->GetLoadState() == LOAD_STATE_LOADED)
        return true;
    
    if (count == 0) return true;

    int loading = 0;
    bool error = false;
    for (int i = 0; i < count; ++i)
    {
        CResource* resource = resources[i];
        if (resource == NULL) continue;

        if (resource->IsError())
        {
            error = true;
            continue;
        }

        if (!resource->IsLoaded()) loading++;

        if (resource->GetLoadState() == LOAD_STATE_UNLOADED || 
            resource->GetPriority().GetPriority() != STREAM_PRIORITY_DONT_DESERIALISE)
            LoadResource(resource, STREAM_IMMEDIATE);
    }

    if (loading && !BlockUntilLoaded_(DONT_WAIT_FOR_STREAMING_RESOURCES, resources, count))
        return false;
    
    return !error;
}

void SetLazyGCSpeed(u32 value)
{
    gLazyGCSpeed = value;
}

MAKE_THREAD_FUNCTION(MainSlowThread)
{
    while (!CSRsForSlow->Aborted())
    {
        CP<CSerialisedResource> csr;
        int priority;

        if (!CSRsForSlow->Pop(priority, csr, -1))
            continue;

        if (WantQuit())
        {
            SetResourceError(csr, (EResourceLoadState)REFLECT_APPLICATION_QUITTING);
            continue;
        }

        CHash latest = csr->GetDescriptor().LatestHash();
        bool has_source = false;
        if (csr->LoosePath.IsEmpty())
        {
            SResourceReader reader;
            if (GetResourceReader(latest, reader) && FileLoad(reader, csr->Data))
                has_source = true;
        }
        else
        {
            has_source = FileLoad(csr->LoosePath, csr->Data, NULL);
        }

        if (has_source)
        {
            AddCSRToDoneQueue(csr, priority);
        }
        else
        {
            SetResourceError(csr, LOAD_STATE_ERROR_FILENOTFOUND);
        }
    }

    THREAD_RETURN(0);
}

MAKE_THREAD_FUNCTION(MainLoadingThread)
{
    while (!CSRsForStaging->Aborted())
    {
        CP<CSerialisedResource> csr;
        int priority;

        if (!CSRsForStaging->Pop(priority, csr, -1))
            continue;

        if (WantQuit())
        {
            SetResourceError(csr, (EResourceLoadState)REFLECT_APPLICATION_QUITTING);
            continue;
        }

        bool has_source = false;
        CCache* last_cache = NULL;
        
        CHash latest = csr->GetDescriptor().LatestHash();
        StringifyHash bytes(latest);

        while (!WantQuit())
        {
            SResourceReader reader;
            if (!GetResourceReader(csr->GetDescriptor(), reader, csr->LoosePath))
                break;
            
            if (GetPreferredSerialisationType(csr->GetDescriptor().GetType()) == PREFER_FILE)
            {
                if (!csr->LoosePath.IsEmpty())
                {
                    has_source = true;
                    AddCSRToDoneQueue(csr, priority);
                }

                break;
            }
            else if (reader.Owner != NULL && reader.Owner->IsSlow(reader))
            {
                FileClose(reader);
                AddCSRToQueue(csr, CSRsForSlow, priority);
                has_source = true;
                break;
            }

            if (FileLoad(reader, csr->Data))
            {
                has_source = true;
                AddCSRToDoneQueue(csr, priority);
                break;
            }

            if ((last_cache != NULL && last_cache == reader.Owner) || has_source) break;


            MMLogCh(DC_RESOURCE, "************ resource cache gave us bad bytes (%s). trying again\n", bytes.c_str());
            last_cache = reader.Owner;
        }

        if (!has_source && !csr->LoosePath.IsEmpty())
        {
            has_source = true;
            AddCSRToQueue(csr, CSRsForSlow, priority);
        }

        if (!has_source)
        {
            MMLogCh(DC_RESOURCE, "%f: Load from network fail, not ready :(\n", GetClockSeconds());
            puts("RESOURCE ERROR - LOAD_STATE_ERROR_NO_DATA_SOURCE");
            SetResourceError(csr, LOAD_STATE_ERROR_NO_DATA_SOURCE);
        }
    }

    MMLogCh(DC_RESOURCE, "Leaving main load thread\n");
    THREAD_RETURN(0);
}

MAKE_THREAD_FUNCTION(ResourceSystemPumpThread)
{
    // gPumpThreadID = GetCurrentThreadId();
    // while (!CSRsDoneThreaded->Aborted())
    // {
    //     ResourceSystemPumpThreadLastTimeUpdated = GetClock();
    //     ResourceSystemPumpWork(CSRsDoneThreaded, -1);
    // }

    // gPumpThreadID = 0;

    THREAD_RETURN(0);
}

const char* ResourceTypeString(EResourceType type)
{
    switch (type)
    {
        case RTYPE_INVALID: return "RTYPE_INVALID";
#define RESOURCE_MACRO(type, class_name, headers) case type: return #class_name;
#define RESOURCE_MACRO_UNIMPLEMENTED RESOURCE_MACRO
    #include <ResourceList.h>
#undef RESOURCE_MACRO_UNIMPLEMENTED
#undef RESOURCE_MACRO
        default: return "(unknown resource type)";
    }
}

THREAD TMainLoadingThread;
THREAD TMainSlowThread;
THREAD TResourcePumpThread;

bool InitResourceSystem()
{
    if (gResourceSystemInited) return true;

    delete gResourceCS;
    gResourceCS = NULL;

    gResourceCS = new CCriticalSec("Resource");

    FileDB::Init();

    CSRsForStaging = new CSRQueue();
    CSRsForSlow = new CSRQueue();
    CSRsForRNP = new CSRQueue();
    CSRsForHTTP = new CSRQueue();
    CSRsDone = new CSRQueue();
    CSRsDoneThreaded = new CSRQueue();
    CSRsFinished = new CSRQueue();

    TMainLoadingThread = ThreadCreate(MainLoadingThread, NULL, "main loading thread", 1000, 0x10000, true);
    TMainSlowThread = ThreadCreate(MainSlowThread, NULL, "main slow thread", 1000, 0x10000, true);
    TResourcePumpThread = ThreadCreate(ResourceSystemPumpThread, NULL, "res pump thread", 1000, 0x10000, true);

    gResourceSystemInited = true;
    return true;
}

void CloseResourceSystem()
{
    if (!gResourceSystemInited) return;

    CSRsForStaging->Abort();
    CSRsForSlow->Abort();
    CSRsForRNP->Abort();
    CSRsForHTTP->Abort();
    CSRsDone->Abort();
    CSRsDoneThreaded->Abort();
    CSRsFinished->Abort();

    ThreadJoin(TMainLoadingThread, NULL);
    ThreadJoin(TMainSlowThread, NULL);
    ThreadJoin(TResourcePumpThread, NULL);
    
    UnloadAllResources();

    delete gResourceCS;
    gResourceCS = NULL;

    delete CSRsForStaging;
    CSRsForStaging = NULL;

    delete CSRsForRNP;
    CSRsForRNP = NULL;

    delete CSRsForHTTP;
    CSRsForHTTP = NULL;

    delete CSRsForStaging;
    CSRsForStaging = NULL;

    delete CSRsDone;
    CSRsDone = NULL;

    delete CSRsDoneThreaded;
    CSRsDoneThreaded = NULL;

    delete CSRsFinished;
    CSRsFinished = NULL;
}

#else // RESOURCE_SYSTEM_REIMPLEMENTATION

ResourceLoadSettings gLoadSettings;
Ib_DefinePort(LoadResourceByDescriptor, CP<CResource>, const CResourceDescriptorBase& desc, const ResourceLoadSettings& settings);
Ib_DefinePort(BlockUntilResourcesLoaded, bool, CResource** resources, u32 count);
Ib_DefinePort(UnloadResource, void, CP<CResource>);
Ib_DefinePort(UnloadUnusedResources, void, bool);
Ib_DefinePort(SerialiseResourceToMemory, ReflectReturn, ByteArray& out, CResource* resource, CSerialiseControlParams const& params, CHash* out_hash);

CP<CResource> LoadResource(const CResourceDescriptorBase& desc, const ResourceLoadSettings& settings)
{
    return LoadResourceByDescriptor(desc, settings);
}

#endif // RESOURCE_SYSTEM_REIMPLEMENTATION