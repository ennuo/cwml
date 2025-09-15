#pragma once

#include <thread.h>
#include <fifo.h>

typedef void (*JobFn)(void*);

class CJobManager { // 12
private:
    struct SJob {
        u32 JobID;
        JobFn Function;
        void* UserData;
        u32 Tag;
        s32 Priority;
    };

    struct STagMatches
    {
        u32 Tag;
        inline STagMatches(u32 tag) : Tag(tag) {}
        inline bool operator()(const SJob& job) const { return job.Tag == Tag; }
    };

    struct SJobIDMatches
    {
        u32 JobID;
        inline SJobIDMatches(u32 id) : JobID(id) {}
        inline bool operator()(const SJob& job) const
        {
            return job.JobID == JobID;
        }
    };
public:
    CJobManager(u32 num_threads);
    ~CJobManager();
public:
    u32 GetUniqueTag();
    void AbortJobsForShutdown();
    u32 EnqueueJob(int prio, JobFn fn, void* userdata, u32 tag, const char* name);
    u32 CountJobsWithJobID(u32 id);
    u32 CountJobsWithTag(u32 tag);
public: 
    static MAKE_THREAD_FUNCTION(WorkerThreadFunctionStatic);
    void WorkerThreadFunction();
private:
    volatile u32 NextUniqueTag;
    volatile u32 NextUniqueJobID;
    CRawVector<THREAD> Threads;
    CPriorityQueue<SJob> WorkQueue;
    CPriorityQueue<SJob> ActiveQueue;
};

extern CJobManager* gJobManager;
extern CJobManager* gHTTPJobManager;

