#pragma once

#ifdef WIN32
    #include <win32/thread.h>
    #define ThreadCreate CreateWin32Thread
    #define ThreadJoin JoinWin32Thread
#elif defined PS3
    #include <cell/thread.h>
    #define ThreadCreate CreatePPUThread
    #define GetCurrentThreadId GetCurrentPPUThreadID
    #define GetCurrentThread GetCurrentPPUThread
    #define ThreadJoin JoinPPUThread
    #define SetThreadPriority SetPPUThreadPriority
#endif

bool AmInMainThread();
bool InitThreads();
