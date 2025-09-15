#include <fifo.h>

CMMSemaphore::CMMSemaphore(int icount, int imaxcount) : Sem(), Abort()
{
    Sem = CreateSemaphore(
        NULL,
        icount,
        imaxcount > 0 ? imaxcount : INT_MAX,
        NULL
    );
}

CMMSemaphore::~CMMSemaphore()
{
    if (Sem != NULL)
    {
        CloseHandle(Sem);
    }
}

bool CMMSemaphore::Increment()
{
    return Increment(1);
}

bool CMMSemaphore::Increment(u32 value)
{
    if (!Abort && ReleaseSemaphore(Sem, value, NULL))
        return !Abort;
    return false;
}

#include <DebugLog.h>

bool CMMSemaphore::WaitAndDecrement(int timeout)
{
    int result = WaitForSingleObject(Sem, timeout != -1 ? timeout : INFINITE);
    if (result != WAIT_OBJECT_0) return false;
    return !Abort;
}

void CMMSemaphore::DoAbort()
{
    if (Abort == false)
    {
        ReleaseSemaphore(Sem, 1000, NULL);
        Abort = true;
    }
}

bool CMMSemaphore::Aborted() const
{
    return Abort;
}
