/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Frame.cpp
 *  @brief  Exposes Naali framework's update tick.
 * 
 *  Frame object can be used to:
 *  -retrieve signal every time frame has been processed
 *  -retrieve the wall clock time of Framework
 *  -to trigger delayed signals when spesified amount of time has elapsed.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Frame.h"

#include <QTimer>

#include "MemoryLeakCheck.h"

DelayedSignal::DelayedSignal(boost::uint64_t startTime) : startTime_(startTime)
{
}

void DelayedSignal::Expire()
{
    emit Triggered((float)((GetCurrentClockTime() - startTime_) / GetCurrentClockFreq()));
}

Frame::Frame(Foundation::Framework *framework) : QObject(framework)
{
    startTime_ = GetCurrentClockTime();
}

Frame::~Frame()
{
    qDeleteAll(delayedSignals_);
}

float Frame::GetWallClockTime() const
{
    return (GetCurrentClockTime() - startTime_) / GetCurrentClockFreq();
}

DelayedSignal *Frame::DelayedExecute(float time)
{
    DelayedSignal *delayed = new DelayedSignal(GetCurrentClockTime());
    QTimer::singleShot(time*1000, delayed, SLOT(Expire()));
    connect(delayed, SIGNAL(Triggered(float)), SLOT(DeleteDelayedSignal()));
    delayedSignals_.push_back(delayed);
    return delayed;
}

void Frame::DelayedExecute(float time, const QObject *receiver, const char *member)
{
    DelayedSignal *delayed = new DelayedSignal(GetCurrentClockTime());
    QTimer::singleShot(time*1000, delayed, SLOT(Expire()));
    connect(delayed, SIGNAL(Triggered(float)), receiver, member);
    connect(delayed, SIGNAL(Triggered(float)), SLOT(DeleteDelayedSignal()));
    delayedSignals_.push_back(delayed);
}

void Frame::Update(float frametime)
{
    emit Updated(frametime);
}

void Frame::DeleteDelayedSignal()
{
    DelayedSignal *expiredSignal = checked_static_cast<DelayedSignal *>(sender());
    assert(expiredSignal);
    delayedSignals_.removeOne(expiredSignal);
    SAFE_DELETE_LATER(expiredSignal);
}
