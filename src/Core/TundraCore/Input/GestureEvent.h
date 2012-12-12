// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"

#include <QObject>
#include <QString>
#include <QGesture>

/// Event data structure passed as the parameter in all in-scene Gesture events.
class TUNDRACORE_API GestureEvent : public QObject
{
    Q_OBJECT
    Q_ENUMS(EventType)
    Q_PROPERTY(EventType eventType READ Type)
    Q_PROPERTY(QGesture* gesture READ Gesture)
    Q_PROPERTY(Qt::GestureType gestureType READ GestureType)

public:
    GestureEvent() :
        eventType(GestureInvalid),
        handled(false),
        gesture(0)
    {
    }

    virtual ~GestureEvent() {}

    enum EventType
    {
        GestureInvalid = 0, ///< An invalid event. Used to check for improperly filled structure.
        GestureStarted = 1, ///< The gestures start event
        GestureUpdated = 2, ///< The gestures update event
        GestureFinished = 3, ///< The gestures finish event
        GestureCanceled = 4 ///< The gestures canceled event, if the event was canceled while it was ongoing
    };

    /// Pointer to the qt gesture class
    QGesture *gesture;

    /// Type of the gesture ptr for casts into the correct subclass 
    /// http://doc.trolltech.com/4.6/qgesture.html
    Qt::GestureType gestureType;

    /// Identifies of the gesture state this event is
    EventType eventType;

    /// This field is used as an accept/suppression flag. When you are handling this event, settings this to true signals that
    /// your module handled this keyevent, and it should not be passed on to Qt for further processing. Of course you can
    /// leave this as false even after you handle this event, in case you don't care whether Qt gets this event as well or not.
    /// By default, this field is set to false when the event is fired to the event queue.
    bool handled;

    EventType Type() const { return eventType; }

public slots:
    /// Marks this event as having been handled already, which will suppress this event from
    /// going on to lower input context levels.
    void Suppress() { handled = true; }
    void Accept() { handled = true; }

    QGesture *Gesture() const { return gesture; } ///< @todo Doesn't need to be a slot; exposed as Q_PROPERTY
    Qt::GestureType GestureType() const { return gestureType; } ///< @todo Doesn't need to be a slot; exposed as Q_PROPERTY
    bool IsStartedEvent() const { return eventType == GestureStarted; } /**< @deprecated Use Type or 'eventType' @todo Remove. */
    bool IsUpdatedEvent() const { return eventType == GestureUpdated; } /**< @deprecated Use Type or 'eventType' @todo Remove. */
    bool IsFinishedEvent() const { return eventType == GestureFinished || eventType == GestureCanceled; } /**< @deprecated Use Type or 'eventType' @todo Remove. */
};
