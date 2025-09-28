#include <CritSec.h>
#include <StringUtil.h>

#include <sys/synchronization.h>
#include <sys/return_code.h>

CCriticalSec::CCriticalSec() : 
#ifndef ANONYMOUS_CRITICAL_SECTIONS
    LockFile(), LockLine(),
#endif
cs(), Name(), DEBUGIsLocked()
{

}

CCriticalSec::CCriticalSec(const char* name)
{
    sys_lwmutex_attribute_t attr;
    attr.name[0] = '\0';
    attr.attr_protocol = SYS_SYNC_FIFO;
    attr.attr_recursive = SYS_SYNC_RECURSIVE;
    
    StringCopy<char, SYS_SYNC_NAME_SIZE>(attr.name, name);

    sys_lwmutex_create(&cs, &attr);

#ifdef LBP1
	Name = name;
#else
	StringCopy<char, 16>(Name, name);
#endif

    DEBUGIsLocked = 0;
#ifndef ANONYMOUS_CRITICAL_SECTIONS
    LockLine = -1;
    LockFile = NULL;
#endif
}

CCriticalSec::~CCriticalSec()
{
    sys_lwmutex_destroy(&cs);
}

void CCriticalSec::Enter(const char* lock_file, int lock_line)
{
    bool printed = false;
    while (true) 
    {
        int res = sys_lwmutex_lock(&cs, 2000000);
        if (res != ETIMEDOUT)
        {
            if (res == CELL_OK) 
            {
#ifndef ANONYMOUS_CRITICAL_SECTIONS
                LockFile = lock_file;
                LockLine = lock_line;
#endif
                DEBUGIsLocked++;
            }

            break;
        }

        if (!printed)
        {
            printf("delay acquiring mutex '%s' %08x retval = %08x\n", this->Name, this->cs.sleep_queue, ETIMEDOUT);
            printed = true;
        }
    }
}

void CCriticalSec::Leave()
{
    DEBUGIsLocked--;
    sys_lwmutex_unlock(&cs);
}
