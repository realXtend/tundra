// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "Win.h"

#include <QDateTime>

#if defined(_POSIX_C_SOURCE) && !defined(Q_WS_MAC)
#include <time.h>
#endif

#ifdef Q_WS_MAC
#include <sys/time.h>
#include <mach/mach_host.h>
#include <mach/clock.h>
#include <mach/mach_init.h>
#endif

typedef u64 tick_t;

inline tick_t TUNDRACORE_API GetCurrentClockTime()
{
#if defined(_WINDOWS)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return *(tick_t*)&now;
#elif defined(ANDROID)
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000000000ULL*res.tv_sec + (tick_t)res.tv_nsec;
#elif defined(_POSIX_MONOTONIC_CLOCK) && !defined(Q_WS_MAC)
    struct timespec clock_now;
    clock_gettime(CLOCK_MONOTONIC, &clock_now);
    return (tick_t) clock_now.tv_sec*1000000000ULL + (tick_t) clock_now.tv_nsec;
#elif defined(_POSIX_C_SOURCE) && !defined(Q_WS_MAC)
    struct timeval clock_now;
    gettimeofday(&clock_now, NULL);
    return (tick_t) clock_now.tv_sec*1000000ULL + (tick_t) clock_now.tv_usec;
#elif defined(Q_WS_MAC)
    struct mach_timespec clock_now;
    clock_serv_t host_clock;
    kern_return_t status = host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &host_clock);
    clock_get_time(host_clock, &clock_now);
    return (tick_t) clock_now.tv_sec * 1000000000ULL + (tick_t) clock_now.tv_nsec;
#else    
    uint now = QDateTime::currentDateTime().toTime_t();
    return now;
#endif
}

inline tick_t TUNDRACORE_API GetCurrentClockFreq()
{
#ifdef _WINDOWS
    LARGE_INTEGER now;
    QueryPerformanceFrequency(&now);
    return *(tick_t*)&now;
#elif defined(ANDROID)
    return 1000000000ULL; // 1e9 == nanoseconds.
#elif defined(_POSIX_MONOTONIC_CLOCK) && !defined(Q_WS_MAC)
    return 1000000000;
#elif defined(_POSIX_C_SOURCE) && !defined(Q_WS_MAC)
    return 1000000;
#elif defined(Q_WS_MAC)
    return 1000000000;
#else
    return 1;
#endif
}
