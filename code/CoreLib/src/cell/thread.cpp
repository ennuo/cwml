#include <sys/timer.h>
#include <cell/thread.h>

bool gGameHostThreadInit;
THREAD gGameHostThreadID;
THREADID gMainThreadID;
bool gThreadInit;


THREAD CreatePPUThread(THREADPROC threadproc, uint64_t thread_arg, const char* name, int priority, int stacksize, bool joinable) 
{
    THREAD rv;
	int ret = sys_ppu_thread_create(
		&rv,
		threadproc,
		thread_arg,
		priority,
		stacksize,
		joinable,
		name
	);
	if (ret == CELL_OK)
		return rv;
	return 0;
}

THREAD GetCurrentPPUThread() 
{
    THREAD rv;
    sys_ppu_thread_get_id(&rv);
    return rv;
}

THREADID GetCurrentPPUThreadID() 
{
    return GetCurrentPPUThread();
}

bool SetPPUThreadPriority(THREAD thread, int priority) 
{
    return sys_ppu_thread_set_priority(thread, priority);
}

bool JoinPPUThread(THREAD t, uint64_t* thread_retval) 
{
    if (t != 0)
        return sys_ppu_thread_join(t, thread_retval);
    return false;
}

void ExitPPUThread(uint64_t retval) 
{
    sys_ppu_thread_exit(retval);
}

void ThreadSleep(int ms) 
{
    if (ms == 0) 
    {
        sys_ppu_thread_yield();
        return;
    }
    
    sys_timer_usleep(ms * 1000);
}

void ThreadSleepUS(int us) 
{
    if (us == 0) 
    {
        sys_ppu_thread_yield();
        return;
    }

    sys_timer_usleep(us);
}

bool AmInMainThread()
{
    THREADID thread_id;
    sys_ppu_thread_get_id(&thread_id);

    if (gThreadInit)
        return thread_id == gMainThreadID;
    
    return true;
}

bool InitThreads()
{
    if (!gThreadInit)
    {
        gThreadInit = true;
        return sys_ppu_thread_get_id(&gMainThreadID) == CELL_OK;
    }

    return true;
}
