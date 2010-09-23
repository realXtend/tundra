/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Frame.h
 *  @brief  Exposes Naali framework's update tick.
 * 
 *  Frame object can be used to:
 *  -retrieve signal every time frame has been processed
 *  -etrieve the wall clock time of Framework
 *  -to trigger delayed signals when spesified amount of time has elapsed.
 */

#ifndef incl_Foundation_Frame
#define incl_Foundation_Frame

#include <QObject>

#include <boost/cstdint.hpp>

namespace Foundation
{
    class Framework;
}

/// Class to which scripting languages connect their slots when wanting to receive delayed signal
/// when certain amount of application time has passed.
/** In C++ you can ignore the existence of this class, and just give your slot to Frame's DelayedExecute
    as a parameter. This class cannot be created directly, it's created by Frame
*/
class DelayedSignal : public QObject
{
    Q_OBJECT

    friend class Frame;

signals:
    /** @brief Emitted when delayed signal is triggered.
        @param time Elapsed framework wall clock time.
    */
    void Triggered(float time);

private:
    /** Construct new signal delayed signal object.
        @param startTime Application tick.
    */
    explicit DelayedSignal(boost::uint64_t startTime);

    /// Application tick when object was created.
    boost::uint64_t startTime_;

private slots:
    /** @brief Emit Triggered() signal
        Called by Frame object when the spesified amount of time has passed.
    */
    void Expire();
};

/// Exposes Naali framework's update tick.
/** This class cannot be created directly, it's created by Framework.
    Frame object can be used to:
    -retrieve signal every time frame has been processed
    -etrieve the wall clock time of Framework
    -to trigger delayed signals when spesified amount of time has elapsed.
*/
class Frame : public QObject
{
    Q_OBJECT

    friend class Foundation::Framework;

    /// Destructor.
    ~Frame();

public slots:
    /// Return wall clock time of Framework in seconds.
    float GetWallClockTime() const;

    /// Triggers DelayedSignal::Triggered(float) signal when spesified amount of time has elapsed.
    /** This function is basically a wrapper for QTimer and provided for convenience for scripting languages
        so that we don't have to expose QTimer manually to e.g.Javascript.
        @param time Time in seconds.
        @note Never returns null pointer
        @note Never store the returned pointer.
    */
    DelayedSignal *DelayedExecute(float time);

    /// Triggers DelayedSignal::Triggered(float) signal when spesified amount of time has elapsed.
    /** Use this function from C++ or in Python if the receiver is a QObject.
        @param time Time in seconds.
        @param receiver Receiver object.
        @param member Member slot.
    */
    void DelayedExecute(float time, const QObject *receiver, const char *member);

signals:
    /** Emitted after one frame is processed.
        @param frametime Elapsed time in seconds since the last frame.
    */
    void Updated(float frametime);

private:
    /** Constuctor.
        @param framework Framework
    */
    explicit Frame(Foundation::Framework *framework);

    /** @brief Increases the wall clock time and emits Updated() signal.
        Called by Framework each frame.
    */
    void Update(float frametime);

    /// Wall clock time of Framework;
    boost::uint64_t startTime_;

    /// Delayed signals.
    QList<DelayedSignal *> delayedSignals_;

private slots:
    /// Deletes delayed signal object and removes it from the list when it's expired.
    void DeleteDelayedSignal();
};

#endif
