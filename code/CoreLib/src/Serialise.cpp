#include <Serialise.h>
#include <CompressionJob.h>
#include <JobManager.h>
#include <thread.h>

const u16 CompressionBufferSize = 0x8000;
const u16 CompressionVersion = 1;

u32 gMinRevision = gFormatRevision;

BranchDefine gBranchDefines[] =
{
    { "Leerdammer", gLeerdammerFormatRevision, gLeerdammerBranchDescription }
};

ReflectReturn SRevision::CheckRevision() const
{
    if (Revision > gFormatRevision) return REFLECT_FORMAT_TOO_NEW;
    return IsAfterRevision(gMinSafeRevision) 
        ? REFLECT_OK : REFLECT_FORMAT_TOO_OLD;
}

ReflectReturn SRevision::CheckBranchDescription() const
{
    if (BranchDescription == 0) return REFLECT_OK;

    for (u32 i = 0; i < sizeof(gBranchDefines); ++i)
    {
        BranchDefine& branch = gBranchDefines[i];
        u16 id = branch.BranchDescription >> 16;
        u16 revision = branch.BranchDescription & 0xffff; 
        if (id == GetBranchID() && GetBranchRevision() <= revision)
            return REFLECT_OK;
    }

    return REFLECT_FORMAT_TOO_NEW;
}

CReflectionBase::CReflectionBase() : NumVisited(),
CanVisitThings(), LazyCPPriority(STREAM_PRIORITY_DEFAULT), World(),
ThingPtrAsUID(), TakenReflectionCS(), TempString(), DependencyCollector()
{

}

CReflectionBase::~CReflectionBase()
{

}

CReflectionVisitLoad::CReflectionVisitLoad() : Visited()
{

}

CReflectionLoadVector::CReflectionLoadVector(const CBaseVector<char>* vec) :
CReflectionBase(), CReflectionVisitLoad(), Vec(vec), Allocated(0), DecryptedVec(), Revision(0, 0), 
CompressionFlags(), JobsTag(), LoadPos(), OutstandingBuffers(), CurJob(),
PendingInit(), Sem(), SPUDecompressAvailable(false)
{

}

CReflectionLoadVector::~CReflectionLoadVector()
{
    delete Sem;
}

CMMSemaphore& CReflectionLoadVector::GetSem()
{
    if (Sem != NULL) return *Sem;
    Sem = new CMMSemaphore(0, 0);
    return *Sem;
}

ReflectReturn CReflectionLoadVector::ReadWrite(void* d, int size)
{
    if (!CurJob)
    {
        if (LoadPos + size <= Vec->size())
        {
            memcpy(d, Vec->begin() + LoadPos, size);
            LoadPos += size;
            return REFLECT_OK;
        }

        return REFLECT_EXCESSIVE_DATA;
    }

    ReflectReturn ret;
    if ((ret = WaitForHead()) != REFLECT_OK) return ret;

    while (size > 0)
    {
        int n = MIN_MACRO(size, CurJob->GetBytesRemaining());
        if (n == 0)
        {
            if (CurJob->IsCompressed())
                OutstandingBuffers -= CurJob->GetBufferSize();
            CurJob = CurJob->Next;
            PumpDecompression();
            if ((ret = WaitForHead()) != REFLECT_OK) 
                return ret;
            continue;
        }

        CurJob->GetData(d, n);

        size -= n;
        d = (void*)(((char*)d) + n);
    }

    return REFLECT_OK;
}

ReflectReturn CReflectionLoadVector::WaitForHead() // 251
{
    ReflectReturn ret = REFLECT_OK;

    int cycles = 0;
    while (ret == REFLECT_OK)
    {
        if (!CurJob || CurJob->GetState() == CJS_DONE)
            break;

        if (!SPUDecompressAvailable && GetSem().WaitAndDecrement(1)) cycles++;

        if (CurJob->IsFinished())
        {
            ret = CurJob->Finalise();
            break;
        }

        if (!SPUDecompressAvailable) ThreadSleepUS(300);

        ret = PumpDecompression();
    }

    if (cycles > 1)
        GetSem().Increment(cycles - 1);

    if (ret == REFLECT_OK && !CurJob)
        return REFLECT_DECOMPRESSION_FAIL;
    
    return ret;
}

// supposed to be in SpugeZlib.cpp:24
bool IsSpuUncompressAvailable() { return false; }

typedef std::pair<u16, u16> SizesPair;

ReflectReturn CReflectionLoadVector::LoadCompressionData(u32* totalsize)
{
    u16 version, num_compressed_blocks;
    ReflectReturn ret;
    if ((ret = Reflect(*this, version)) != REFLECT_OK) return ret;
    if ((ret = Reflect(*this, num_compressed_blocks)) != REFLECT_OK) return ret;

    u32 tmp;
    if (totalsize == NULL) totalsize = &tmp;
    *totalsize = 0;

    if (num_compressed_blocks == 0) return REFLECT_OK;
    JobsTag = gJobManager->GetUniqueTag();

    u32 startpos = LoadPos + num_compressed_blocks * sizeof(SizesPair);
    u32 compressedsize = 0;
    u32 size = 0;

    CP<DecompressionJob> first;
    DecompressionJob* last;

    for (int i = 0; i < num_compressed_blocks; ++i)
    {
        SizesPair sizes;
        if ((ret = Reflect(*this, sizes)) != REFLECT_OK) return ret;

        u16 stored_size = sizes.first;
        u16 original_size = sizes.second;

        if (stored_size == 0 || original_size == 0)
            return REFLECT_DECOMPRESSION_FAIL;

        u32 load_pos = startpos + compressedsize;
        DecompressionJob* next = new DecompressionJob(&GetSem(), Vec, load_pos, stored_size, original_size);;
        
        if (!first) first = next;
        else last->Next = next;
        last = next;

        compressedsize += stored_size;
        size += original_size;

        *totalsize += original_size;
    }

    if (LoadPos != startpos) return REFLECT_DECOMPRESSION_FAIL;

    SPUDecompressAvailable = IsSpuUncompressAvailable();
    LoadPos = startpos + compressedsize;
    
    CurJob = first;
    PendingInit = first;

    PumpDecompression();
    return REFLECT_OK;
}

ReflectReturn CReflectionLoadVector::CleanupDecompression()
{
    ReflectReturn rv = CurJob != NULL && CurJob->GetBytesRemaining() == 0 
        ? REFLECT_OK : REFLECT_DECOMPRESSION_FAIL;
    
    while (CurJob != NULL)
        CurJob = CurJob->Next;

    while (PendingInit != NULL)
        PendingInit = PendingInit->Next;
    
    return REFLECT_OK;

    return rv;
}

ReflectReturn CReflectionLoadVector::PumpDecompression() // 400
{
    while (OutstandingBuffers < 0x80000)
    {
        if (!PendingInit) return REFLECT_OK;

        if (PendingInit->GetState() == CJS_BEGIN)
        {
            OutstandingBuffers += PendingInit->GetBufferSize();
            PendingInit->EnqueueForDecompress(SPUDecompressAvailable, JobsTag);
        }

        PendingInit = PendingInit->Next;
    }

    return REFLECT_OK;
}