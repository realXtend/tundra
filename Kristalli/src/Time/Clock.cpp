/** @file Clock.cpp
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Clock supplies platform-independent timing.
*/
#include "clb/Time/Clock.h"

namespace clb
{

int Clock::Year()
{
	return ClockImpl::Year();
}

int Clock::Month()
{
	return ClockImpl::Month();
}

int Clock::Day()
{
	return ClockImpl::Day();
}

int Clock::Hour()
{
	return ClockImpl::Hour();
}

int Clock::Min()
{
	return ClockImpl::Min();
}

int Clock::Sec()
{
	return ClockImpl::Sec();
}

unsigned long Clock::SystemTime()
{
	return ClockImpl::SystemTime();
}

unsigned long Clock::Time()
{
	return ClockImpl::Time();
}

/** Note that depending on the resolution, the counter may overflow quite fast.
	Thus, when using it to calculate per-frame time differences, remember to take
	this into consideration. */
tick_t Clock::Tick()
{
	return ClockImpl::Tick();
}

unsigned long Clock::TickU32()
{
	return ClockImpl::TickU32();
}

tick_t Clock::TicksPerSec()
{
	return ClockImpl::TicksPerSec();
}

}
