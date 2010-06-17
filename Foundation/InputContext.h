// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_InputContext_h
#define incl_QtInputModule_InputContext_h

#include <map>
#include <set>

#include "QtInputKeyEvent.h"
#include "QtInputMouseEvent.h"

#include "KeyEventSignal.h"

struct KeyPressInformation
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

class InputContext : public QObject
{
    Q_OBJECT

signals:
    /// Emitted for each key code, for each event type.
    void OnKeyEvent(KeyEvent &key);
    /// Emitted for each mouse event (move, scroll, button press/release).
    void OnMouseEvent(MouseEvent &mouse);

    /// This signal is emitted when any key is pressed in this context.
    void KeyPressed(KeyEvent &key);
    /// This signal is emitted for each application frame when this key is pressed down in this context.
    void KeyDown(KeyEvent &key);
    /// This signal is emitted when any key is released in this context.
    void KeyReleased(KeyEvent &key);

    /// Emitted when the mouse cursor is moved, independent of whether any buttons are down.
    void MouseMove(MouseEvent &mouse);
    /// Mouse wheel was scrolled.
    void MouseScroll(MouseEvent &mouse);

    // The following signals are emitted on the appropriate events. It is guaranteed that each press event
    // will follow a corresponding release event (although if it gets lost from Qt, it might get delayed
    // until we actually notice it).
    void MouseLeftPressed(MouseEvent &mouse);
    void MouseMiddlePressed(MouseEvent &mouse);
    void MouseRightPressed(MouseEvent &mouse);

    void MouseLeftReleased(MouseEvent &mouse);
    void MouseMiddleReleased(MouseEvent &mouse);
    void MouseRightReleased(MouseEvent &mouse);

public:
    explicit InputContext(const char *name);

    /// By registering to InputContext::KeyPressed, you can receive a signal
    /// directly for a given keyCode, without having to register to OnKeyEvent
    /// for all keys, and then if()'ing if you had the correct one you were
    /// interested about.
    KeyEventSignal &RegisterKeyEvent(Qt::Key keyCode);
    void UnregisterKeyEvent(Qt::Key keyCode);

    /// This function is called by the QtInputService whenever there is a new
    /// key event for this context to handle. The event is emitted through
    /// all relevant signals. You may call this function yourself to inject 
    /// keyboard events to this context manually.
    void TriggerKeyEvent(KeyEvent &key);

    /// Constructs a key release event for the given key and calls TriggerKeyEvent for it.
    /// Use to forcibly release a key that is being held down.
    void TriggerKeyReleaseEvent(Qt::Key keyCode);

    /// Same as TriggerKeyEvent, but for mouse events.
    void TriggerMouseEvent(MouseEvent &mouse);

    /// Returns the user-defined name of this InputContext. The name is
    /// read-only, and associated with the context at creation time.
    std::string Name() const { return name; }

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

    /// Updates the buffered key presses. Called by the input service to
    /// proceed on to the next input frame.
    void UpdateFrame();

    /// Forces all held down keys to be released, and the appropriate release events to be sent.
    void ReleaseAllKeys();

private:
    typedef std::map<Qt::Key, KeyEventSignal> KeyEventSignalMap;
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

    std::string name;

    // InputContexts are noncopyable.
    InputContext(const InputContext &);
    void operator=(const InputContext &);
};

typedef boost::shared_ptr<InputContext> InputContextPtr;

#endif
