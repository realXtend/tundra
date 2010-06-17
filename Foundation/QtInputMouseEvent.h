// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_MouseEvent_h
#define incl_QtInputModule_MouseEvent_h

#include "CoreTypes.h"
#include "EventDataInterface.h"

#include <QNamespace.h>
#include <QPoint>

namespace QtInputEvents
{
    // These are the mouse-related input events provided by the QtInputModule.
    // For input events on keyboard, see \see KeyEvent.h.

	static const event_id_t MousePressed = 3;
	static const event_id_t MouseReleased = 4;
	static const event_id_t MouseClicked = 5;
	static const event_id_t MouseDoubleClicked = 6;
	static const event_id_t MouseMove = 7;
	static const event_id_t MouseScroll = 8;
}

/// MouseEvent is the event data structure passed as the parameter in all Naali mouse-related events.
class MouseEvent : public Foundation::EventDataInterface
{
public:
	MouseEvent()
    :eventType(MouseEventInvalid),
    button(MouseEvent::NoButton),
    origin(PressOriginNone),
    x(-1),y(-1),z(-1),
    relativeX(-1),relativeY(-1),relativeZ(-1),
    globalX(-1),globalY(-1),
    otherButtons(0),
    handled(false)
    {
    }
	virtual ~MouseEvent() {}

    /// These correspond to Qt::MouseEvent enum. See http://doc.trolltech.com/4.6/qt.html#MouseButton-enum 
	enum MouseButton
	{
        NoButton = 0,
		LeftButton = 1,
		RightButton = 2,
		MiddleButton = 4,
		Button4 = 8,
		Button5 = 16,
        MaxButtonMask = 32
	};

	enum EventType
	{
        MouseEventInvalid, ///< An invalid event. Used to check for improperly filled structure.
		MouseMove, ///< The mouse cursor moved. This event is passed independent of which buttons were down.
		MouseScroll, ///< The mouse wheel position moved.
		MousePressed, ///< A mouse button was pressed down.
		MouseReleased, ///< A mouse button was released.
///\todo Offer these additional events.
//		MouseClicked,  ///< A mouse click occurs when mouse button is pressed and released inside a short interval.
//		MouseDoubleClicked
	};

    /// PressOrigin tells whether a mouse press originated on top of a Qt widget or on top of the 3D scene area.
    enum PressOrigin
    {
        PressOriginNone, ///< No press of the given type has been registered.
        PressOriginScene,
        PressOriginQtWidget
    };

	EventType eventType;

	/// The button that this event corresponds to.
	MouseButton button;

    /// If eventType==MousePress, this field tells whether the click originates on the 3D scene or on a Qt widget.
    PressOrigin origin;

	// The mouse coordinates in the client coordinate area.
	int x;
	int y;
	/// The mouse wheel absolute position.
	int z;

	// The difference that occurred during this event and the previous one.
	int relativeX;
	int relativeY;
	// The difference in the mouse wheel position (for most mouses).
	int relativeZ;

	// The global mouse coordinates where the event occurred.
	int globalX;
	int globalY;

	/// What other mouse buttons were held down when this event occurred. This is a combination of MouseButton flags.
	unsigned long otherButtons;

	/// Which keyboard keys were held down when this event occurred.
	std::vector<Qt::Key> heldKeys;

    /// The coordinates in window client coordinate space denoting where the mouse left [0] /middle [1] /right [2] /XButton1 [3] /XButton2 [4] 
    /// buttons were pressed down. These are useful in mouse drag situations where it is necessary to know the coordinates where the 
    /// mouse dragging started.
    struct PressPositions
    {
        PressPositions();

        PressOrigin origin[5];

        int x[5];
        int y[5];

        /// Returns the mouse coordinates in local client coordinate frame denoting where the given mouse button was last pressed
        /// down. Note that this does not tell whether the mouse button is currently held down or not.
        QPoint Pos(int mouseButton) const;
        PressOrigin Origin(int mouseButton) const;

        void Set(int mouseButton, const QPoint &pt, PressOrigin origin) { Set(mouseButton, pt.x(), pt.y(), origin); }
        void Set(int mouseButton, int x, int y, PressOrigin origin);
    };

    PressPositions mousePressPositions;

    /// This field is used as an accept/suppression flag. When you are handling this event, settings this to true signals that
    /// your module handled this event, and it should not be passed on to Qt for further processing. Of course you can
    /// leave this as false even after you handle this event, in case you don't care whether Qt gets this event as well or not.
    /// By default, this field is set to false when the event is fired to the event queue.
    bool handled;

	///\todo Add a time stamp of the event.

	/// @return True if the key with the given keycode was held down when this event occurred.
	bool HadKeyDown(int keyCode) const { return std::find(heldKeys.begin(), heldKeys.end(), keyCode) != heldKeys.end(); }

    /// Returns the mouse coordinates in local client coordinate frame denoting where the given mouse button was last pressed
    /// down. Note that this does not tell whether the mouse button is currently held down or not.
    QPoint MousePressedPos(int mouseButton) const;

    /// Returns true if the given mouse button is held down in this event.
    bool IsButtonDown(MouseButton button_) const;

    // Conveniency functions for above. Note that these do not tell if this event was a mouse press event for the given button,
    // only that the button was detected to be down during the time this event was generated.
    bool IsLeftButtonDown() const { return IsButtonDown(LeftButton); }
    bool IsMiddleButtonDown() const { return IsButtonDown(MiddleButton); }
    bool IsRightButtonDown() const { return IsButtonDown(RightButton); }
/*
    ///\todo Implement these.
	bool HasShiftModifier() const { return (modifiers & Qt::ShiftModifier) != 0; }
	bool HasCtrlModifier() const { return (modifiers & Qt::CtrlModifier) != 0; }
	bool HasAltModifier() const { return (modifiers & Qt::AltModifier) != 0; }
	/// On windows, this is associated to the Win key.
	bool HasMetaModifier() const { return (modifiers & Qt::MetaModifier) != 0; }
*/
};

#endif
