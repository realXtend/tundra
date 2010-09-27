// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_HighPerfClock_h
#define incl_Core_HighPerfClock_h

#include <QDateTime>

#if defined(_POSIX_C_SOURCE)
#include <time.h>
#endif

#include <boost/cstdint.hpp>

typedef boost::uint64_t tick_t;

inline tick_t GetCurrentClockTime()
{
#if defined(_WINDOWS)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return *(tick_t*)&now;
#elif defined(_POSIX_MONOTONIC_CLOCK)
    struct timespec clock_now;
    clock_gettime(CLOCK_MONOTONIC, &clock_now);
    return (tick_t) clock_now.tv_sec*1000000000ULL + (tick_t) clock_now.tv_nsec;
#elif defined(_POSIX_C_SOURCE)
    struct timeval clock_now;
    gettimeofday(&clock_now, NULL);
    return (tick_t) clock_now.tv_sec*1000000ULL + (tick_t) clock_now.tv_usec;
#else
    uint now = QDateTime::currentDateTime().toTime_t();
    return now;
#endif
}

inline tick_t GetCurrentClockFreq()
{
#ifdef _WINDOWS
    LARGE_INTEGER now;
    QueryPerformanceFrequency(&now);
    return *(tick_t*)&now;
#elif defined(_POSIX_MONOTONIC_CLOCK)
    return 1000000000;
#elif defined(_POSIX_C_SOURCE)
    return 1000000;
#else
    return 1;
#endif
}

#endif
