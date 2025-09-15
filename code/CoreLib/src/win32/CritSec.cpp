// This file doesn't technically exist, on PS3
// it just uses CritSec.h and inlines all calls,
// but I'd prefer not to pollute the scope with
// bullshit from windows.h

#include <CritSec.h>
#include <StringUtil.h>

#include <Windows.h>

CCriticalSec::CCriticalSec() : 
cs(), Name(), LockFile(), LockLine(), DEBUGIsLocked()
{

}

CCriticalSec::CCriticalSec(const char* name) : cs()
{
    InitializeCriticalSection(&cs);

#ifdef LBP1
	Name = name;
#else
	StringCopy<char, 16>(Name, name);
#endif

    DEBUGIsLocked = 0;
    LockLine = -1;
    LockFile = NULL;
}

CCriticalSec::~CCriticalSec()
{
    DeleteCriticalSection(&cs);
}

void CCriticalSec::Enter(const char* lock_file, int lock_line)
{
    EnterCriticalSection(&cs);
    DEBUGIsLocked++;
}

void CCriticalSec::Leave()
{
    DEBUGIsLocked--;
    LeaveCriticalSection(&cs);
}
