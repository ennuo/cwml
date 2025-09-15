#pragma once

u64 GetClock();
u64 GetClockFreq();
float GetClockFreqInv();
float GetClockSeconds();
float GetClockMilliSeconds();
float GetClockMicroSeconds();
void InitClock();
u64 SecondsToClockTicks(float seconds);
u64 GetSystemClock();
u64 GetClockMicroSecondsInt();
u64 GetClockMilliSecondsInt();
float ToSeconds(u64 clocktime);
float ToMilliSeconds(u64 clocktime);
bool InitPerformanceTimers();


