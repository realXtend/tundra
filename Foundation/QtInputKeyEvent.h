// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_KeyEvent_h
#define incl_QtInputModule_KeyEvent_h

#include "CoreTypes.h"
#include "IEventData.h"

#include <QObject>
#include <QString>
#include <QKeySequence>

namespace QtInputEvents
{
	static const event_id_t KeyPressed = 1;
	static const event_id_t KeyReleased = 2;
}

/// KeyEvent is the event data structure passed as the parameter in all Naali in-scene KeyPressed and KeyReleased events.
class KeyEvent : public QObject, public IEventData
{
    Q_OBJECT
    Q_ENUMS(EventType)

public:
    KeyEvent()
    :keyCode((Qt::Key)0),
    keyPressCount(0),
    modifiers(0),
    eventType(KeyEventInvalid),
    handled(false)
    {
    }

	virtual ~KeyEvent() {}

	/// The key code associated with this key event.
    /// See Qt::Key from http://doc.trolltech.com/4.6/qt.html#Key-enum
    /// \note This member stores the pressed key without keyboard modifiers attached.
    Qt::Key keyCode;

	/// How many times this key event has been pressed during the time the key has been held down. If 1, this means a new 
    /// keypress. If >1, this means that this event is being raised on a key repeat event, and this field increments by
    /// one for each time the repeat signal is received.
    /// \note It seems that Qt sends repeats OK for character keys, but it never repeats modifier keys.
	int keyPressCount;

    /// A bitfield of the keyboard modifiers (Ctrl, Shift, ...) associated with this key event.
    /// Use Qt::KeyboardModifier, http://doc.trolltech.com/4.6/qt.html#KeyboardModifier-enum to access these.
    /// Also see \see KeyEvent::HasShiftModifier.
	unsigned long modifiers;

	enum EventType
	{
        KeyEventInvalid, ///< An invalid event. Used to check for improperly filled structure.
		KeyPressed, ///< The key was pressed, or repeated by key repeat. Read the keyPressCount field to distinguish between these.
        KeyDown, ///< The key is being held down. This event is generated once per frame. \todo Not yet implemented.
		KeyReleased, ///< The key was released.
	};

    /// Identifies whether this was a key press or a release event.
	EventType eventType;

	/// The readable textual representation associated with this event, if applicable.
	QString text;

    /// The key sequence that has been produced by this keypress.
    QKeySequence sequence;

	/// Contains Qt's keycodes for all other keys that are being pressed when this key was pressed.
    /// See Qt::Key, http://doc.trolltech.com/4.6/qt.html#Key-enum
	/// This member is only valid when eventType==KeyPressed.
    std::vector<Qt::Key> otherHeldKeys;

    /// This field is used as an accept/suppression flag. When you are handling this event, settings this to true signals that
    /// your module handled this keyevent, and it should not be passed on to Qt for further processing. Of course you can
    /// leave this as false even after you handle this event, in case you don't care whether Qt gets this event as well or not.
    /// By default, this field is set to false when the event is fired to the event queue.
    bool handled;

	///\todo Add a time stamp of the event.
	///\todo Add hold duration if this is a release/repeated press.

public slots:
    /// Marks this event as having been handled already, which will suppress this event from
    /// going on to lower input context levels.
    void Suppress() { handled = true; }

    /// Returns true if this event represents a repeated keypress.
    bool IsRepeat() const { return eventType == KeyPressed && keyPressCount > 1; }

    /// A conveniency accessor to otherHeldKeys field.
	/// @return True if the key with the given keycode was held down when this event occurred.
	bool HadKeyDown(int keyCode) const { return std::find(otherHeldKeys.begin(), otherHeldKeys.end(), keyCode) != otherHeldKeys.end(); }

    // Conveniency accessors for the keyboard modifiers.
	bool HasShiftModifier() const { return (modifiers & Qt::ShiftModifier) != 0; }
	bool HasCtrlModifier() const { return (modifiers & Qt::ControlModifier) != 0; }
	bool HasAltModifier() const { return (modifiers & Qt::AltModifier) != 0; }
	/// On windows, this is associated with the Win key.
	bool HasMetaModifier() const { return (modifiers & Qt::MetaModifier) != 0; }

    /// Returns the pressed key with the modifier bits in it.
    int KeyWithModifier() const { return (int)keyCode | (int)modifiers; }
public:
    // Meta-information wrappers for dynamic languages.
    Q_PROPERTY(QKeySequence sequence READ Sequence)
    Q_PROPERTY(QString text READ Text)
    Q_PROPERTY(unsigned long modifiers READ Modifiers)
    Q_PROPERTY(int keyPressCount READ KeyPressCount)
    Q_PROPERTY(Qt::Key keyCode READ KeyCode)

    QKeySequence Sequence() const { return sequence; }
    QString Text() const { return text; }
    unsigned long Modifiers() const { return modifiers; }
    int KeyPressCount() const { return keyPressCount; }
    Qt::Key KeyCode() const { return keyCode; }

};

#endif
