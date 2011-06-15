/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   FrameAPI.h
 *  @brief  Frame core API. Exposes framework's update tick.
 */

#pragma once

#include "CoreTypes.h"

#include <QObject>

class Framework;

/// Stores a delayed signal invocation.
/** Scripting languages connect their slots when wanting to receive delayed signal when certain amount of application time has passed.
    In C++ you can ignore the existence of this class, and just give your slot to FrameAPI's DelayedExecute
    as a parameter. This class cannot be created directly, it's created by FrameAPI. */
class DelayedSignal : public QObject
{
    Q_OBJECT

    friend class FrameAPI;

signals:
    /// Emitted when delayed signal is triggered.
    /** @param time Elapsed framework wall clock time. */
    void Triggered(float time);

private:
    /// Construct new signal delayed signal object.
    /** @param startTime Application tick. */
    explicit DelayedSignal(u64 startTime);

    u64 startTime; ///< Application tick when object was created.

private slots:
    /// Emits Triggered() signal
    /** Called by FrameAPI object when the spesified amount of time has passed. */
    void Expire();
};

/// Provides a mechanism for plugins and scripts to receive per-frame and time-based events.
/** This class cannot be created directly, it's created by Framework.
    FrameAPI object can be used to:
    -retrieve signal every time frame has been processed
    -retrieve the wall clock time of Framework
    -trigger delayed signals when spesified amount of time has elapsed. */
class FrameAPI : public QObject
{
    Q_OBJECT

    friend class Framework;

    ~FrameAPI();

    /// Clears all registered signals to this API.
    void Reset();

public slots:
    /// Return wall clock time of Framework in seconds.
    float WallClockTime() const;

    /// Triggers DelayedSignal::Triggered(float) signal when spesified amount of time has elapsed.
    /** This function is basically a wrapper for QTimer and provided for convenience for scripting languages
        so that we don't have to expose QTimer manually to e.g.Javascript.
        @param time Time in seconds.
        @note Never returns null pointer
        @note Never store the returned pointer. */
    DelayedSignal *DelayedExecute(float time);

    /// Triggers DelayedSignal::Triggered(float) signal when spesified amount of time has elapsed.
    /** Use this function from C++ or in Python if the receiver is a QObject.
        @param time Time in seconds.
        @param receiver Receiver object.
        @param member Member slot. */
    void DelayedExecute(float time, const QObject *receiver, const char *member);

    /// Returns the current application frame number.
    /** @note It is best not to tie any timing-specific animation to this number, but instead use GetWallClockTime(). */
    int FrameNumber() const;

signals:
    /// Emitted after one frame is processed.
    /** @param frametime Elapsed time in seconds since the last frame. */
    void Updated(float frametime);

private:
    /// Constructor. Framework takes ownership of this object.
    /** @param framework Framework */
    explicit FrameAPI(Framework *framework);

    /// Emits Updated() signal. Called by Framework each frame.
    /** @param frametime Time elapsed since last frame. */
    void Update(float frametime);

    u64 startTime; ///< Start time time of Framework/this object;
    QList<DelayedSignal *> delayedSignals; ///< Delayed signals.
    int currentFrameNumber;

private slots:
    /// Deletes delayed signal object and removes it from the list when it's expired.
    void DeleteDelayedSignal();
};
