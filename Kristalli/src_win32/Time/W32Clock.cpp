/** @file W32Clock.cpp
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Win32 -clock implementation.
*/
#include "clb/Core/clbAssert.h"
#include "clb/Time/Clock.h"

#define NOMINMAX
#include <windows.h>

namespace clb
{
namespace win
{
W32Clock impl;

LARGE_INTEGER W32Clock::ddwTimerFrequency;
unsigned long W32Clock::appStartTime = 0xFFFFFFFF;

void W32Clock::InitClockData()
{
	if (appStartTime != 0xFFFFFFFF)
		return;

	if (!QueryPerformanceFrequency(&ddwTimerFrequency))
	{
		LOGWARNING("The system doesn't support high-resolution timers!");
		ddwTimerFrequency.HighPart = (unsigned long)-1;
		ddwTimerFrequency.LowPart = (unsigned long)-1;
	}

	if (ddwTimerFrequency.HighPart > 0)
		LOGWARNING("Clock::TicksPerSec will yield invalid timing data!");

	if (appStartTime == 0)
		appStartTime = GetTickCount();
}

W32Clock::W32Clock()
{
	InitClockData();
}

int W32Clock::Year()
{
	SYSTEMTIME s;
	GetSystemTime(&s);
	return s.wYear;
}

int W32Clock::Month()
{
	SYSTEMTIME s;
	GetSystemTime(&s);
	return s.wMonth;
}

int W32Clock::Day()
{
	SYSTEMTIME s;
	GetSystemTime(&s);
	return s.wDay;
}

int W32Clock::Hour()
{
	SYSTEMTIME s;
	GetSystemTime(&s);
	return s.wHour;
}

int W32Clock::Min()
{
	SYSTEMTIME s;
	GetSystemTime(&s);
	return s.wMinute;
}

int W32Clock::Sec()
{
	SYSTEMTIME s;
	GetSystemTime(&s);
	return s.wSecond;
}

unsigned long W32Clock::SystemTime()
{
	return GetTickCount();
}

unsigned long W32Clock::Time()
{
	return GetTickCount() - appStartTime;
}

tick_t W32Clock::Tick()
{
	LARGE_INTEGER ddwTimer;
	QueryPerformanceCounter(&ddwTimer);
	return ddwTimer.QuadPart;
}

unsigned long W32Clock::TickU32()
{
	LARGE_INTEGER ddwTimer;
	QueryPerformanceCounter(&ddwTimer);
	return ddwTimer.LowPart;
}

tick_t W32Clock::TicksPerSec()
{
	return ddwTimerFrequency.QuadPart;
}

}
}
