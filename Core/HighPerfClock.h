// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Core_HighPerfClock_h
#define incl_Core_HighPerfClock_h

#include <QDateTime>

namespace Core
{

typedef boost::uint64_t tick_t;

inline tick_t GetCurrentClockTime()
{
#ifdef _WINDOWS
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return *(tick_t*)&now;
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
#else
    return 1;
#endif
}

}

#endif

