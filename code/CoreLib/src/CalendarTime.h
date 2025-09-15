#pragma once

#include <time.h>
typedef time_t CalendarTime;

CalendarTime GetCalendarTime();
bool TryGetCalendarTime(CalendarTime& timestamp);
u64 GetCalendarMicroseconds();