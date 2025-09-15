#include <win32/thread.h>

bool gGameHostThreadInit;
THREAD gGameHostThreadID;
THREADID gMainThreadID;
bool gThreadInit;

THREAD CreateWin32Thread(THREADPROC threadproc, uint64_t thread_arg, const char* name, int priority, int stacksize, bool joinable)
{
    return CreateThread(
        NULL,
        stacksize,
        threadproc,
        (LPVOID)thread_arg,
        0,
        NULL
    );
}

bool JoinWin32Thread(THREAD t, uint64_t* thread_retval)
{
    WaitForSingleObject(t, INFINITE);
    
    if (thread_retval != NULL)
    {
        DWORD retval;
        if (GetExitCodeThread(t, &retval))
            *thread_retval = retval;
    }

    // CloseHandle(t);
    
    return true;
}

void ThreadSleep(int ms)
{
    if (ms == 0)
    {
        SwitchToThread();
        return;
    }

    Sleep(ms);
}

void ThreadSleepUS(int us)
{
    ThreadSleep(us / 1000);
}

bool AmInMainThread()
{
    THREADID thread_id = GetCurrentThreadId();
    if (gThreadInit)
        return thread_id == gMainThreadID;
    return true;
}

bool InitThreads()
{
    if (!gThreadInit)
    {
        gThreadInit = true;
        gMainThreadID =  GetCurrentThreadId();
    }

    return true;
}