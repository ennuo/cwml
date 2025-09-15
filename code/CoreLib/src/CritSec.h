#pragma once

#ifdef WIN32
	#include <synchapi.h>
#else
	#include <sys/synchronization.h>
#endif 

class CCriticalSec {
public:
	CCriticalSec();
	CCriticalSec(const char* name);
	~CCriticalSec();
public:
	void Enter(const char* lock_file, int lock_line);
	void Leave();
private:
#ifdef WIN32
	CRITICAL_SECTION cs;
#else
	sys_lwmutex_t cs;
#endif
#ifdef LBP1
    const char* Name;
#else
    char Name[16];
#endif
    const char* LockFile;
    int LockLine;
    int DEBUGIsLocked;
};

class CCSLock {
public:
	inline CCSLock(CCriticalSec* c, const char* lock_file, int lock_line) 
    {
		CS = c;
		if (CS != NULL)
			CS->Enter(lock_file, lock_line);
	}
    
	inline ~CCSLock() { if (CS != NULL) CS->Leave();  }
	inline void Set(CCriticalSec* c) { CS = c; }
private:
    CCriticalSec* CS;
};
