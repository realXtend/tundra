// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_KeyEvent_h
#define incl_QtInputModule_KeyEvent_h

#include "QtInputModuleApi.h"
#include "CoreTypes.h"
#include "EventDataInterface.h"

#include <QString>
#include <QKeySequence>

namespace QtInputEvents
{
	static const event_id_t KeyPressed = 1;
	static const event_id_t KeyReleased = 2;
}

/// KeyEvent is the event data structure passed as the parameter in all Naali in-scene KeyPressed and KeyReleased events.
class QTINPUT_MODULE_API KeyEvent : public Foundation::EventDataInterface
{
public:
    KeyEvent()
    :keyCode((Qt::Key)0),
    repeatCount(0),
    modifiers(0),
    eventType(KeyEventInvalid),
    handled(false)
    {
    }

	virtual ~KeyEvent() {}

	/// The key code associated with this key event.
    /// See Qt::Key from http://doc.trolltech.com/4.6/qt.html#Key-enum
    Qt::Key keyCode;

	/// How many times this key event has been repeated. If 0, this means a new keypress. If 1, this means that this event
    /// is being raised on the first repeat event, i.e. the second time.
    /// \note It seems that Qt sends repeats OK for character keys, but it never repeats modifier keys.
	int repeatCount;

    /// A bitfield of the keyboard modifiers (Ctrl, Shift, ...) associated with this key event.
    /// Use Qt::KeyboardModifier, http://doc.trolltech.com/4.6/qt.html#KeyboardModifier-enum to access these.
    /// Also see \see KeyEvent::HasShiftModifier.
	unsigned long modifiers;

	enum EventType
	{
        KeyEventInvalid, ///< An invalid event. Used to check for improperly filled structure.
		KeyPressed, ///< The key was pressed, repeated by keyrepeat. Read the repeatCount field to distinguish between these.
        KeyDown, ///< The key is being held down. This event is generated once per frame.
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

    /// A conveniency accessor to otherHeldKeys field.
	/// @return True if the key with the given keycode was held down when this event occurred.
	bool HadKeyDown(int keyCode) const { return std::find(otherHeldKeys.begin(), otherHeldKeys.end(), keyCode) != otherHeldKeys.end(); }

    // Conveniency accessors for the keyboard modifiers.
	bool HasShiftModifier() const { return (modifiers & Qt::ShiftModifier) != 0; }
	bool HasCtrlModifier() const { return (modifiers & Qt::ControlModifier) != 0; }
	bool HasAltModifier() const { return (modifiers & Qt::AltModifier) != 0; }
	/// On windows, this is associated with the Win key.
	bool HasMetaModifier() const { return (modifiers & Qt::MetaModifier) != 0; }
};

#endif
