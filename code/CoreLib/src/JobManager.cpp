#include <JobManager.h>

#ifdef WIN32
    #include <windows.h>
    #define INCREMENT_32(v) InterlockedIncrement((LONG*)&v) - 1;
#else
    #include <cell/atomic.h>
    #define INCREMENT_32(v) cellAtomicIncr32((uint32_t*) &v);
#endif

CJobManager* gJobManager;
CJobManager* gHTTPJobManager;

#include <DebugLog.h>

CJobManager::CJobManager(u32 num_threads) : NextUniqueTag(1), NextUniqueJobID(1),
Threads(), WorkQueue(), ActiveQueue() // 7
{
    for (int i = 0; i < num_threads; ++i)
        Threads.push_back(ThreadCreate(WorkerThreadFunctionStatic, (uint64_t)this, "JobManagerWorker", 1000, 0x10000, true));
}

CJobManager::~CJobManager() // 24
{
    WorkQueue.Abort();
    ThreadSleep(100);
    for (int i = 0; i < Threads.size(); ++i)
        ThreadJoin(Threads[i], NULL);
}

void CJobManager::AbortJobsForShutdown() // 40
{

}

u32 CJobManager::GetUniqueTag() // 47
{
    return INCREMENT_32(NextUniqueTag) + 1;
}

u32 CJobManager::EnqueueJob(int prio, JobFn fn, void* userdata, u32 tag, const char* name) // 52
{
    SJob job;

    job.Priority = prio;
    job.JobID = INCREMENT_32(NextUniqueJobID) + 1;
    job.JobID = NextUniqueJobID + 1;
    job.Function = fn;
    job.UserData = userdata;
    job.Tag = tag;

    ActiveQueue.Push(191 - prio, job);
    WorkQueue.Push(191 - prio, job);

    return job.JobID;
}

u32 CJobManager::CountJobsWithJobID(u32 id) // 69
{
    return ActiveQueue.Count(SJobIDMatches(id));
}

u32 CJobManager::CountJobsWithTag(u32 tag) // 75
{
    return ActiveQueue.Count(STagMatches(tag));
}

MAKE_THREAD_FUNCTION(CJobManager::WorkerThreadFunctionStatic) // 82
{
    ((CJobManager*)arg)->WorkerThreadFunction();
    THREAD_RETURN(0);
}

void CJobManager::WorkerThreadFunction() // 89
{
    while (!WorkQueue.Aborted())
    {
        THREAD thread = GetCurrentThread();
        int prio; SJob job;
        if (!WorkQueue.Pop(prio, job, -1)) continue;
        SetThreadPriority(thread, 191 - prio);
        job.Function(job.UserData);
        ActiveQueue.Erase(SJobIDMatches(job.JobID));
        SetThreadPriority(thread, 1000);
    }
}