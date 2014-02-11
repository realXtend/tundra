// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "Time/Clock.h"

/// Returns Clock::Tick()
/// @todo 11.02.2014 Unnecessary wrapper nowadays. Remove and migrate to using Clock::Tick everywhere instead.
inline tick_t TUNDRACORE_API GetCurrentClockTime() { return Clock::Tick(); }
/// Returns Clock::TicksPerSec()
/// @todo 11.02.2014 Unnecessary wrapper nowadays. Remove and migrate to using Clock::TicksPerSec everywhere instead.
inline tick_t TUNDRACORE_API GetCurrentClockFreq() { return Clock::TicksPerSec(); }
