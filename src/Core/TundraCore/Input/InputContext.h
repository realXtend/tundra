// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "KeyEvent.h"
#include "KeyEventSignal.h"
#include "InputFwd.h"

#include <map>
#include <set>

#include <QCursor>

/// Holds information about pressed key.
struct TUNDRACORE_API KeyPressInformation
{
    /// Identifies the press count for the key. 0 denotes the key is not being held down.
    /// 1 means that the key is being held down, and no key repeat signals have yet occurred.
    /// A value > 1 means that the key has been held down for a longer period, and this field
    /// tells how many repeated presses have been received already for the key.
    int keyPressCount;

    /// Specifies in which state the key is in.
    KeyEvent::EventType keyState;

    /// The absolute timestamp (in seconds) telling when the key was first pressed down.
    f64 firstPressTime;
};
typedef std::map<Qt::Key, KeyPressInformation> HeldKeysMap;

/// Provides clients with input events in a priority order.
class TUNDRACORE_API InputContext : public QObject, public enable_shared_from_this<InputContext>
{
    Q_OBJECT

public:
    InputContext(InputAPI *owner, const char *name, int priority);

    ~InputContext();

    /// Updates the buffered key presses. Called by the input API to proceed on to the next input frame.
    void UpdateFrame();

signals:
    /// Emitted for each key code, for each event type.
    void KeyEventReceived(KeyEvent *key);

    /// Emitted for each mouse event (move, scroll, button press/release).
    void MouseEventReceived(MouseEvent *mouse);

    /// Emitted for every gesture event (started, updated, finished and canceled)
    void GestureEventReceived(GestureEvent *gesture);

    /// This signal is emitted when any key is pressed in this context.
    void KeyPressed(KeyEvent *key);

    /// This signal is emitted for each application frame when this key is pressed down in this context.
    void KeyDown(KeyEvent *key);

    /// This signal is emitted when any key is released in this context.
    void KeyReleased(KeyEvent *key);

    /// Emitted when the mouse cursor is moved, independent of whether any buttons are down.
    void MouseMove(MouseEvent *mouse);

    /// Mouse wheel was scrolled.
    void MouseScroll(MouseEvent *mouse);

    /// Mouse double click
    void MouseDoubleClicked(MouseEvent *mouse);

    // The following signals are emitted on the appropriate events. It is guaranteed that each press event
    // will follow a corresponding release event (although if it gets lost from Qt, it might get delayed
    // until we actually notice it).
    void MouseLeftPressed(MouseEvent *mouse);
    void MouseMiddlePressed(MouseEvent *mouse);
    void MouseRightPressed(MouseEvent *mouse);

    void MouseLeftReleased(MouseEvent *mouse);
    void MouseMiddleReleased(MouseEvent *mouse);
    void MouseRightReleased(MouseEvent *mouse);

    /// @note You need to accept the started event using Accept() in order to receive update and finished events.
    void GestureStarted(GestureEvent *gesture);

    /// @note You need to accept the started event in order to receive update events.
    void GestureUpdated(GestureEvent *gesture);

    /// @note You need to accept the started event in order to receive update events.
    void GestureFinished(GestureEvent *gesture);

public slots:
    /// Creates a new signal object that will be triggered when the given
    /// key sequence occurs in this context. To actually receive the events,
    /// connect to one of the member signals of the returned object.
    /// You do not need to hold on to the received signal
    /// object, it will be remembered and freed along with the context.
    KeyEventSignal &RegisterKeyEvent(QKeySequence keySequence);

    /// Stops signals from being triggered for the given key sequence in this context.
    /// This is optional. KeyEventSignals are freed properly when the context is destroyed.
    void UnregisterKeyEvent(QKeySequence keySequence);

    /// This function is called by the InpuAPI whenever there is a new
    /// key event for this context to handle. The event is emitted through
    /// all relevant signals. You may call this function yourself to inject 
    /// keyboard events to this context manually.
    void TriggerKeyEvent(KeyEvent &key);

    /// Constructs a key release event for the given key and calls TriggerKeyEvent for it.
    /// Use to forcibly release a key that is being held down.
    void TriggerKeyReleaseEvent(Qt::Key keyCode);

    /// Same as TriggerKeyEvent, but for mouse events.
    void TriggerMouseEvent(MouseEvent &mouse);

    // Trigger gesture event signals
    void TriggerGestureEvent(GestureEvent &gesture);

    /// Returns the user-defined name of this InputContext. The name cannot be used as an unique identifier, it is only
    /// present for human-readable purposes.
    QString Name() const { return name; }

    /// Sets the name of this InputContext. The name cannot be used as an unique identifier, it is only
    /// present for human-readable purposes.
    void SetName(const QString &name_) { name = name_; }

    /// Adjusts the priority of this input context.
    void SetPriority(int newPriority);

    /// Tests whether the given key was pressed down in this context.
    /// @return The keypress count for the given keycode. If 0, means that
    /// the key was not pressed down during this frame, or that it is being
    /// held down. If 1, it means that the key was pressed down during this
    /// frame. If > 1, it means that key has been held down for a longer
    /// duration, and key repeat was triggered for it this frame. The value
    /// denotes the repeat count.
    int KeyPressedCount(Qt::Key keyCode) const;

    /// A convenience method to test key presses ignoring repeats.
    bool IsKeyPressed(Qt::Key keyCode) const { return KeyPressedCount(keyCode) == 1; }

    /// Returns true if the given key is being held down in this context.
    bool IsKeyDown(Qt::Key keyCode) const;

    /// Returns true if the given key was released in this context during
    /// this frame.
    bool IsKeyReleased(Qt::Key keyCode) const;

    /// Tells the InputContext whether the given keyCode is automatically
    /// suppressed. This can be used to avoid having to manually set the 
    /// handled-flag to true for each received event.
    /// @param keyCode The keyboard code to set the suppression state for.
    /// @param isSuppressed If true, events related to this keycode will 
    /// be automatically suppressed after this context has handled them,
    /// without passing the event on to lower layers.
    void SetKeySuppressed(Qt::Key keyCode, bool isSuppressed);

    /// Set to true to receive mouse input events even when the mouse cursor is over a Qt widget.
    /// By default, this is disabled.
    void SetTakeMouseEventsOverQt(bool take) { takeMouseEventsOverQt = take; }
    bool TakesMouseEventsOverQt() const { return takeMouseEventsOverQt; }

    /// Set to true to receive keyboard input events even when a Qt widget has keyboard focus.
    /// By default, this is disabled.
    void SetTakeKeyboardEventsOverQt(bool take) { takeKeyboardEventsOverQt = take; }
    bool TakesKeyboardEventsOverQt() const { return takeKeyboardEventsOverQt; }

    /// Forces all held down keys to be released, and the appropriate release events to be sent.
    void ReleaseAllKeys();

    /// Returnts the priority value this context has with respect to the other input contexts. 
    /// Higher = more urgent. Used to determine the order in which input is received by the input contexts.
    /// The priority is assigned when the context is created and may not be changed afterwards.
    /// @note A lower priority context may still receive an event "over" a higher priority context,
    /// if that event is going to go to Qt and the higher priority context does not
    /// capture mouse/key events over Qt, but the lover priority context does.
    int Priority() const { return priority; }
    
    /// Convenience function to allow for simple
    /// disconnecting of all signals from Python
    void disconnectAll() { this->disconnect(); }

    /// Specifies the mouse cursor to use in with this input context.
    void SetMouseCursorOverride(QCursor cursor);

    /// Returns the override cursor this context has, or 0 if this context doesn't specify one.
    QCursor *MouseCursorOverride() const;

    /// Removes mouse cursor from being set in this input context.
    void ClearMouseCursorOverride();

private:
    typedef std::map<QKeySequence, KeyEventSignal*> KeyEventSignalMap;
    /// Stores a signal object for each keyboard key code that the user
    /// has registered a signal-slot connection for.
    KeyEventSignalMap registeredKeyEventSignals;

    /// Stores the set of keycodes that this input automatically suppresses, 
    /// i.e. this context "grabs" these keys and does not pass them forward 
    /// to anyone else.
    std::set<Qt::Key> suppressedKeys;
    
    /// Stores each new key event that is being held down.
    HeldKeysMap newKeyEvents;

    /// Stores a buffered version of all the key pressed. This is to avoid losing any press or
    /// release events in the case of different module Update() orders, or when accessing the
    /// context from Qt event handlers outside Update() cycle.
    HeldKeysMap heldKeysBuffered;

    /// Tests both newKeyEvents and heldKeysBuffered and looks if this context is aware of the
    /// given key being pressed down.
    bool IsKeyDownImmediate(Qt::Key keyCode) const;

    /// If true, this context receives mouse events even when the mouse cursor is over a Qt widget. Default: false.
    bool takeMouseEventsOverQt;

    /// If true, this context receives keyboard events even when a QGraphicsItem in the scene has keyboard focus. Default: false.
    bool takeKeyboardEventsOverQt;

    QString name;

    /// The priority value this context has with respect to the other input contexts. Higher = more urgent. Used
    /// to determine the order in which input is received by the input contexts.
    int priority;

    shared_ptr<QCursor> mouseCursorOverride;
   
    InputAPI *inputApi;

    // InputContexts are noncopyable.
    InputContext(const InputContext &);
    void operator=(const InputContext &);

    friend class InputAPI;
};

