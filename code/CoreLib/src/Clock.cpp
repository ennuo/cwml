#include <Clock.h>

#ifdef PS3
    #include <sys/time_util.h>
    #include <sys/sys_time.h>
#endif

#ifdef VITA
    #include <psp2/kernel/processmgr.h>
#endif

#ifdef WIN32

ULONGLONG gStartTime;
u64 GetClock()
{
    ULONGLONG clock = GetTickCount64();
    if (gStartTime == 0)
        gStartTime = clock;
    return clock - gStartTime;
}

float GetClockSeconds()
{
    return GetClock() / 1000.0f;
}

float ToMilliseconds(u64 clocktime)
{
    return (float)clocktime;
}

float ToSeconds(u64 clocktime)
{
    return clocktime / 1000.0f;
}

bool InitPerformanceTimers()
{
    return true;
}

#else


// this will be divergent from the in-games start time
u64 gStartTime;
u64 gClockFreq;
float gClockFreqInv;

void InitClock()
{
#ifdef PS3
    gClockFreq = sys_time_get_timebase_frequency();
    gClockFreqInv = 1.0f / (float)gClockFreq;
#elif defined VITA
    gClockFreq = 1000000;
    gClockFreqInv = 1.0f / (float)gClockFreq;
#endif
}

u64 GetClock()
{
    u64 clock;
#ifdef PS3
    SYS_TIMEBASE_GET(clock);
#elif defined VITA
    clock = sceKernelGetProcessTimeWide();
#endif
    if (gStartTime == 0)
        gStartTime = clock;
    return clock - gStartTime;
}

u64 GetClockFreq()
{
    if (gClockFreq == 0) InitClock();
    return gClockFreq;
}

float GetClockFreqInv()
{
    if (gClockFreq == 0) InitClock();
    return gClockFreqInv;
}

bool InitPerformanceTimers()
{
    if (gClockFreq == 0) InitClock();
    return true;
}

float ToSeconds(u64 clocktime)
{
    return (float)clocktime * GetClockFreqInv();
}

float ToMilliseconds(u64 clocktime)
{
    return (float)clocktime * 1000.0f * GetClockFreqInv();
}

float GetClockSeconds()
{
    return (float)GetClock() * GetClockFreqInv();
}

float GetClockMilliSeconds()
{
    return (float)GetClock() * 1000.0f * GetClockFreqInv();
}

u64 GetClockMilliSecondsInt()
{
    if (gClockFreq == 0) InitClock();
    return GetClock() * 1000 / gClockFreq;
}

#endif // WIN32