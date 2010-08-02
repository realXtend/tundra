/** @file Clock.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Clock supplies platform-independent timing.
*/
#ifndef Clock_h
#define Clock_h

#include "ClockImpl.h"

namespace clb
{

/** @brief High-resolution timing and system time.

	Gives out timing information in various forms. Use this rather than 
	any platform-dependent perf-counters or rdtsc or whatever.*/
class Clock
{
private:
	Clock();
//	~Clock() {}

public:
	/// @return The current year.
	static int Year();

	/// @return The current month, [1,12]
	static int Month();

	/// @return The current day of month, [1,31]
	static int Day();

	/// @return The current hour of day, [0,23]
	static int Hour();

	/// @return The current clock minute, [0,59]
	static int Min();

	/// @return The current clock second, [0,59]
	static int Sec();

	/// @return The current system time counter in milliseconds.
	static unsigned long SystemTime(); 

	/// @return The number of milliseconds since application start.
	static unsigned long Time(); 

	/// @return The low part of the current tick-time (using whatever high-resolution counter available)
	static unsigned long TickU32();

	/// @return The current tick-time (using whatever high-resolution counter available)
	static tick_t Tick();

	/// @return How many ticks make up a second.
	static tick_t TicksPerSec(); 

	static inline tick_t TicksPerMillisecond() { return TicksPerSec() / 1000; }

	static inline tick_t TicksInBetween(tick_t newTick, tick_t oldTick)
	{
		return (tick_t)(newTick - oldTick);
	}

    /// @return true if newTick is newer than oldTick.
	static inline bool IsNewer(tick_t newTick, tick_t oldTick)
	{
		return TicksInBetween(newTick, oldTick) < ((tick_t)(-1) >> 1);
	}

	static inline float TicksToMilliseconds(tick_t ticks) { return ticks * 1000.f / TicksPerSec(); }

	static inline double TicksToSecondsD(tick_t ticks) { return ticks / (double)TicksPerSec(); }

	static inline float TimespanToMilliseconds(tick_t oldTick, tick_t newTick) { return TicksToMilliseconds(TicksInBetween(newTick, oldTick)); }

	static inline double TimespanToSecondsD(tick_t oldTick, tick_t newTick) { return TicksToSecondsD(TicksInBetween(newTick, oldTick)); }
};

}

#endif
