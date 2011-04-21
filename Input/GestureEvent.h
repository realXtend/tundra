// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_GestureEvent_h
#define incl_Input_GestureEvent_h

#include "CoreTypes.h"

#include <QObject>
#include <QString>
#include <QGesture>

namespace QtInputEvents
{
    static const int GestureStarted = 9;
    static const int GestureUpdated = 10;
    static const int GestureFinished = 11;
    static const int GestureCanceled = 11;
}

/// GestureEvent is the event data structure passed as the parameter in all in-scene Gesture events.
class GestureEvent : public QObject
{
    Q_OBJECT
    Q_ENUMS(EventType)

public:
    GestureEvent() :
        eventType(GestureInvalid),
        handled(false)
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

public slots:
    /// Marks this event as having been handled already, which will suppress this event from
    /// going on to lower input context levels.
    void Suppress() { handled = true; }
    void Accept() { handled = true; }

    QGesture *Gesture() const { return gesture; }
    Qt::GestureType GestureType() const { return gestureType; }

    bool IsStartedEvent() { if (eventType == GestureStarted) return true; else return false; }
    bool IsUpdatedEvent() { if (eventType == GestureUpdated) return true; else return false; }
    bool IsFinishedEvent() { if ((eventType == GestureFinished) || (eventType == GestureCanceled)) return true; else return false; }
};

#endif
