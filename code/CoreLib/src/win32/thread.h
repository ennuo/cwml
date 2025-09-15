#pragma once

#include <Windows.h>

typedef HANDLE THREAD;
typedef DWORD THREADID;
typedef LPTHREAD_START_ROUTINE THREADPROC;

#define MAKE_THREAD_FUNCTION(name) DWORD WINAPI name(LPVOID arg)
#define THREAD_RETURN(value) return value

THREAD CreateWin32Thread(THREADPROC threadproc, uint64_t thread_arg, const char* name, int priority, int stacksize, bool joinable);
bool JoinWin32Thread(THREAD t, uint64_t* thread_retval);

void ThreadSleep(int ms);
void ThreadSleepUS(int us);