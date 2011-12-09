/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   FrameAPI.cpp
 *  @brief  Frame core API. Exposes framework's update tick.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "FrameAPI.h"
#include "HighPerfClock.h"
#include "Profiler.h"
#include <QTimer>

#include "MemoryLeakCheck.h"

FrameAPI::FrameAPI(Framework *fw) : QObject(fw), currentFrameNumber(0)
{
    startTime = GetCurrentClockTime();
}

FrameAPI::~FrameAPI()
{
}

void FrameAPI::Reset()
{
    qDeleteAll(delayedSignals);
    delayedSignals.clear();
}

float FrameAPI::WallClockTime() const
{
    return (float)((double)(GetCurrentClockTime() - startTime) / GetCurrentClockFreq());
}

DelayedSignal *FrameAPI::DelayedExecute(float time)
{
    DelayedSignal *delayed = new DelayedSignal(GetCurrentClockTime());
    QTimer::singleShot(time*1000, delayed, SLOT(Expire()));
    connect(delayed, SIGNAL(Triggered(float)), SLOT(DeleteDelayedSignal()));
    delayedSignals.push_back(delayed);
    return delayed;
}

void FrameAPI::DelayedExecute(float time, const QObject *receiver, const char *member)
{
    DelayedSignal *delayed = new DelayedSignal(GetCurrentClockTime());
    QTimer::singleShot(time*1000, delayed, SLOT(Expire()));
    connect(delayed, SIGNAL(Triggered(float)), receiver, member);
    connect(delayed, SIGNAL(Triggered(float)), SLOT(DeleteDelayedSignal()));
    delayedSignals.push_back(delayed);
}

void FrameAPI::Update(float frametime)
{
    PROFILE(FrameAPI_Update);

    emit Updated(frametime);
    emit PostFrameUpdate(frametime);

    ++currentFrameNumber;
    if (currentFrameNumber < 0)
        currentFrameNumber = 0;
}

void FrameAPI::DeleteDelayedSignal()
{
    DelayedSignal *expiredSignal = checked_static_cast<DelayedSignal *>(sender());
    assert(expiredSignal);
    delayedSignals.removeOne(expiredSignal);
    SAFE_DELETE_LATER(expiredSignal);
}

int FrameAPI::FrameNumber() const
{
    return currentFrameNumber;
}

DelayedSignal::DelayedSignal(u64 startTime_) : startTime(startTime_)
{
}

void DelayedSignal::Expire()
{
    emit Triggered((float)((GetCurrentClockTime() - startTime) / GetCurrentClockFreq()));
}
