#include <fifo.h>

CMMSemaphore::CMMSemaphore(int icount, int imaxcount) : Sem(), Abort()
{
    sys_semaphore_attribute_t sem_attr;
    sys_semaphore_create(
        &Sem,
        &sem_attr,
        icount,
        imaxcount > 0 ? imaxcount : INT_MAX
    );
}

CMMSemaphore::~CMMSemaphore()
{
    if (Sem != 0)
    {
        sys_semaphore_destroy(Sem);
        Sem = 0;
    }
}

bool CMMSemaphore::Increment()
{
    return Increment(1);
}

bool CMMSemaphore::Increment(u32 value)
{
    if (!Abort && sys_semaphore_post(Sem, value) == CELL_OK)
        return !Abort;
    return false;
}

bool CMMSemaphore::WaitAndDecrement(int timeout)
{
    int ret;
    if (timeout == 0) ret = sys_semaphore_trywait(Sem);
    else ret = sys_semaphore_wait(Sem, timeout != -1 ? timeout * 1000 : 0);
    if (ret != CELL_OK) return false;
    return !Abort;
}

void CMMSemaphore::DoAbort()
{
    if (!Abort)
    {
        Abort = true;
        sys_semaphore_post(Sem, 1000);
    }
}

bool CMMSemaphore::Aborted() const
{
    return Abort;
}