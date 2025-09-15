#include <CompressionJob.h>
#include <JobManager.h>
#include <memory/Memory.h>
#include <zlib.h>

bool DecompressionJob::IsFinished() const // 170
{
    if (State == CJS_WORKING)
    {
        if (JobID != 0)
            return gJobManager->CountJobsWithJobID(JobID) != 0;
        return UncompressResult->Result != UR_PENDING;
    }

    return State == CJS_DONE;
}

DecompressionJob::DecompressionJob(CMMSemaphore* s, const CBaseVector<char>* v, u32 load_pos, u16 stored_size, u16 original_size) // 142
: CBaseCounted(), StoredSize(stored_size), OriginalSize(original_size), Vector(v), Done(s), LoadPos(load_pos),
Next(), State(OriginalSize == StoredSize ? CJS_DONE : CJS_BEGIN), UncompressResult(),
Buffer(), JobID(), Offset()
{
    UncompressResult = (SUncompressJobResult*)MM::AlignedMalloc(0x80, 0x80);
    UncompressResult->Result = UR_PENDING;
}

DecompressionJob::~DecompressionJob() // 164
{
    MM::AlignedFree(UncompressResult);
    MM::AlignedFree(Buffer);
    Next = NULL;
}

void DecompressionJob::DecompressJob(void* userdata) // 199
{
    DecompressionJob& job = *(DecompressionJob*)userdata;
    uLongf destLen;
    int ret = uncompress((Bytef*)job.Buffer, &destLen, (const Bytef*)(job.Vector->begin() + job.LoadPos), job.StoredSize);

    job.UncompressResult->Result = (ret != Z_OK || job.OriginalSize != destLen) ? UR_COMPLETED_FAILURE : UR_COMPLETED_SUCCESS;
    job.Done->Increment();
}

void DecompressionJob::EnqueueForDecompress(bool spu_available, u32 tag) // 217
{
    Buffer = MM::AlignedMalloc(OriginalSize, 256);
    UncompressResult->Result = UR_PENDING;
    State = CJS_WORKING;

    if (spu_available)
    {
        // nope!
    }

    gJobManager->EnqueueJob(1000, &DecompressJob, (void*)this, tag, "DecompressionJob");
}

void DecompressionJob::GetData(void* data, u32 size)
{
    const char* buffer = (const char*)(IsCompressed() ? Buffer : Vector->begin());
    memcpy(data, buffer + Offset, size);
    Offset += size;
}

ReflectReturn DecompressionJob::Finalise() // 240
{
    State = CJS_DONE;
    return UncompressResult->Result == UR_COMPLETED_SUCCESS 
        ? REFLECT_OK : REFLECT_DECOMPRESSION_FAIL;
}
