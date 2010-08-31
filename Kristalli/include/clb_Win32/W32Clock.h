/** @file W32Clock.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Win32 implementation of Clock.
*/
#ifndef W32Clock_h
#define W32Clock_h

#define NOMINMAX
#include <windows.h>

namespace clb
{

typedef unsigned long long tick_t;

namespace win
{

/// clb::Clock implementation for Windows. Utilizes QueryPerformanceCounter and GetSystemTime.
class W32Clock
{
private:
	static LARGE_INTEGER ddwTimerFrequency; ///< Ticks per second.
	static LARGE_INTEGER ddwTimer;          ///< Temporary storage for Win32 function calls.
	static unsigned long appStartTime;      ///< Application startup time in ticks.

	/// Initializes clock tick frequency and marks the application startup time.
	void InitClockData();

public:
	W32Clock();
//	~W32Clock() {}

	static int Year(); ///< @return The current year, e.g. 2006.
	static int Month(); ///< @return The current month, [1,12].
	static int Day(); ///< @return The current day, [1,31].
	static int Hour(); ///< @return The current hour, [0,23].
	static int Min(); ///< @return The current minute, [0,59].
	static int Sec(); ///< @return The current second, [0,59].

	/// @return The current system time counter in milliseconds.
	static unsigned long SystemTime();

	/// @return The number of milliseconds since application start.
	static unsigned long Time();

	/// @return The current tick (using whatever high-resolution counter available)
	static tick_t Tick(); 

	/// @return The low part (lowest u32) of the current tick (using whatever high-resolution counter available)
	static unsigned long TickU32(); 

	/// @return How many ticks make up a second.
	static tick_t TicksPerSec();
};

}
}

#endif
