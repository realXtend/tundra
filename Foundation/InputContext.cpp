// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"
#include "InputContext.h"

InputContext::InputContext(const char *name_)
:name(name_)
{
}

void InputContext::TriggerKeyEvent(KeyEvent &key)
{
    HeldKeysMap::iterator heldKeyRecord = heldKeys.find(key.keyCode);

    KeyEventSignalMap::iterator keySignal = registeredKeyEventSignals.find(key.keyCode);
    switch(key.eventType)
    {
    case KeyEvent::KeyPressed: 
        // 1. First emit the generic OnKeyEvent signal that receives all event types for all key codes.
        emit OnKeyEvent(key);
        // 2. Emit the event type -specific signal for all key codes.
        emit KeyPressed(key);
        // 3. Emit the key code -specific signal for specific event.
        if (keySignal != registeredKeyEventSignals.end())
            keySignal->second.OnKeyPressed(key);
        break;
    case KeyEvent::KeyDown:
        if (heldKeyRecord == heldKeys.end() || 
            (heldKeyRecord->second.keyState != KeyEvent::KeyDown && heldKeyRecord->second.keyState != KeyEvent::KeyPressed))
            break; // If we've received a keydown for a key we haven't gotten a corresponding press for before, ignore this event.

        emit OnKeyEvent(key); // 1.
        emit KeyDown(key); // 2.
        if (keySignal != registeredKeyEventSignals.end())
            keySignal->second.OnKeyDown(key); // 3.
        break;
    case KeyEvent::KeyReleased:
        if (heldKeyRecord == heldKeys.end() || 
            (heldKeyRecord->second.keyState != KeyEvent::KeyDown && heldKeyRecord->second.keyState != KeyEvent::KeyPressed))
            break; // If we've received a release for a key we haven't gotten a corresponding press for before, ignore this event.

        emit OnKeyEvent(key); // 1.
        emit KeyReleased(key); // 2.
        if (keySignal != registeredKeyEventSignals.end())
            keySignal->second.OnKeyReleased(key); // 3.
        break;
    default:
        assert(false);
        break;
    }

    KeyPressInformation info;
    info.keyState = key.eventType;
    info.keyPressCount = key.keyPressCount;
//    info.firstPressTime = key.firstPressTime;
    heldKeys[key.keyCode] = info;

    // Now if this layer is registered to suppress this keypress from being processed further,
    // mark it handled.
    if (suppressedKeys.find(key.keyCode) != suppressedKeys.end())
        key.handled = true;
}

void InputContext::TriggerMouseEvent(MouseEvent &mouse)
{
    emit OnMouseEvent(mouse);

    switch(mouse.eventType)
    {
    case MouseEvent::MouseMove: emit MouseMove(mouse); break;
    case MouseEvent::MouseScroll: emit MouseScroll(mouse); break;
    case MouseEvent::MousePressed:
        switch(mouse.button)
        {
        case MouseEvent::LeftButton: emit MouseLeftPressed(mouse); break;
        case MouseEvent::RightButton: emit MouseRightPressed(mouse); break;
        case MouseEvent::MiddleButton: emit MouseMiddlePressed(mouse); break;
            ///\todo XButton1 and XButton2 support?
        }
        break;
    case MouseEvent::MouseReleased: 
        switch(mouse.button)
        {
        case MouseEvent::LeftButton: emit MouseLeftReleased(mouse); break;
        case MouseEvent::RightButton: emit MouseRightReleased(mouse); break;
        case MouseEvent::MiddleButton: emit MouseMiddleReleased(mouse); break;
            ///\todo XButton1 and XButton2 support?
        }
        break;
    default:
        assert(false);
        break;
    }
}

int InputContext::KeyPressedCount(Qt::Key keyCode) const
{
    HeldKeysMap::const_iterator iter = heldKeys.find(keyCode);
    if (iter == heldKeys.end())
        return 0; // The key is not being held down.

    if (iter->second.keyState != KeyEvent::KeyPressed && 
        iter->second.keyState != KeyEvent::KeyDown)
        return 0;

    return iter->second.keyPressCount;
}

bool InputContext::IsKeyDown(Qt::Key keyCode) const
{
    HeldKeysMap::const_iterator iter = heldKeys.find(keyCode);
    if (iter == heldKeys.end())
        return false;

    return iter->second.keyState == KeyEvent::KeyPressed || 
        iter->second.keyState == KeyEvent::KeyDown;
}

bool InputContext::IsKeyReleased(Qt::Key keyCode) const
{
    HeldKeysMap::const_iterator iter = heldKeys.find(keyCode);
    if (iter == heldKeys.end())
        return false;

    return iter->second.keyState == KeyEvent::KeyReleased;
}

void InputContext::SetKeySuppressed(Qt::Key keyCode, bool isSuppressed)
{
    if (isSuppressed)
        suppressedKeys.insert(keyCode);
    else
        suppressedKeys.erase(keyCode);
}
