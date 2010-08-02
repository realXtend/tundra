/** @file PolledTimer.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief PolledTimer is a timer object that can be used in "polled" or per-frame executed contexts.
*/
#ifndef PolledTimer_h
#define PolledTimer_h

#include "Clock.h"

namespace clb
{

class PolledTimer
{
	bool enabled;
	tick_t alarmTime;
/*
	bool periodic;
	tick_t period;
*/
public:
	PolledTimer():enabled(false)//, periodic(false)
	{
	}

	explicit PolledTimer(float msecs)
	:enabled(false)
	{
		StartMSecs(msecs);
	}

	/// Starts the timer in a non-periodic mode, to go off once in the given amount of milliseconds.
	void StartMSecs(float msecs)
	{
		StartTicks((tick_t)(Clock::TicksPerSec() * (msecs / 1000.f)));
	}

	/// Starts the timer in a non-periodic mode, to go off once in the given amount of high-precision Clock ticks.
	void StartTicks(tick_t ticks)
	{
		alarmTime = Clock::Tick() + ticks;
		enabled = true;
//		periodic = false;
	}
/*
	/// Starts the the timer in a periodic mode, to go off continuously in the given interval.
	void StartPeriodicMSecs(float msecs)
	{
		StartPeriodicTicks((tick_t)(Clock::TicksPerSec() * (msecs / 1000.f)));
	}

	void StartPeriodicTicks(tick_t ticks)
	{
		alarmTime = Clock::Tick() + ticks;
		enabled = true;
		periodic = true;
		period = ticks;
	}
*/
	void Stop()
	{
		enabled = false;
	}

	void Reset()
	{
//		if (periodic)
//			StartPeriodicTicks(period);
//		else
			Stop();
	}

	bool Enabled() const
	{
		return enabled;
	}

	/// Tests whether the timer has gone off, and resets it as well.
	bool Test()
	{
		if (!enabled)
			return false;
		if (Clock::IsNewer(Clock::Tick(), alarmTime))
		{
			Reset();
			return true;
		}
		return false;
	}

	/// @return The number of ticks left until the timer elapses, or 0xFFFFFFFF if the timer is not active.
	tick_t TicksLeft() const
	{
		if (!enabled)
			return (tick_t)(-1);

		tick_t now = Clock::Tick();
		if (Clock::IsNewer(now, alarmTime))
			return 0;
		else
			return Clock::TicksInBetween(alarmTime, now);
	}

	/// Waits until the timer elapses. Uses OS Sleep() to avoid excessive CPU use.
	void WaitPrecise()
	{
		if (!enabled)
			return;

		tick_t timeLeft = TicksLeft();
		while(timeLeft > 0)
		{
			if (timeLeft > Clock::TicksPerMillisecond())
			{
				float msecs = Clock::TicksToMilliseconds(timeLeft);
				Sleep((int)msecs);
			}
			else // If there's less than one ms left, we use spinwait to return precisely as near to the correct tick as possible.
			{
				SpinWait();
				return;
			}
			timeLeft = TicksLeft();
		}
	}

	/// Waits in an empty loop until the timer elapses. Will cause the the CPU use to go to max.
	void SpinWait()
	{
		while(enabled && TicksLeft() > 0)
			;
	}
};

}

#endif
