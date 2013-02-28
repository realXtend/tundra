// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "InputContext.h"
#include "InputAPI.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "GestureEvent.h"
#include "KeyEventSignal.h"

#include <QList>
#include <QCursor>

#include "MemoryLeakCheck.h"

InputContext::InputContext(InputAPI *owner, const char *name_, int priority_)
:inputApi(owner), name(name_), priority(priority_), takeMouseEventsOverQt(false), takeKeyboardEventsOverQt(false)
{
}

InputContext::~InputContext()
{
    for(KeyEventSignalMap::iterator iter = registeredKeyEventSignals.begin();
        iter != registeredKeyEventSignals.end(); ++iter)
        delete iter->second;
}

KeyEventSignal &InputContext::RegisterKeyEvent(QKeySequence keySequence)
{
    KeyEventSignalMap::iterator iter = registeredKeyEventSignals.find(keySequence);
    if (iter != registeredKeyEventSignals.end())
        return *iter->second;
    KeyEventSignal *signal = new KeyEventSignal(keySequence);
    registeredKeyEventSignals[keySequence] = signal;
    return *signal;
}

void InputContext::UnregisterKeyEvent(QKeySequence keySequence)
{
    KeyEventSignalMap::iterator iter = registeredKeyEventSignals.find(keySequence);
    if (iter != registeredKeyEventSignals.end())
    {
        delete iter->second;
        registeredKeyEventSignals.erase(iter);
    }
}

void InputContext::TriggerKeyEvent(KeyEvent &key)
{
    KeyEventSignalMap::iterator keySignal = registeredKeyEventSignals.find(key.keyCode);
    switch(key.eventType)
    {
    case KeyEvent::KeyPressed:
        // 1. First emit the generic KeyEventReceived signal that receives all event types for all key codes.
        emit KeyEventReceived(&key);
        // 2. Emit the event type -specific signal for all key codes.
        emit KeyPressed(&key);
        // 3. Emit the key code -specific signal for specific event.
        if (keySignal != registeredKeyEventSignals.end())
            keySignal->second->OnKeyPressed(key);
        break;
    case KeyEvent::KeyDown:
        if (!IsKeyDownImmediate(key.keyCode))
            break; // If we've received a keydown for a key we haven't gotten a corresponding press for before, ignore this event.

        emit KeyEventReceived(&key); // 1.
        emit KeyDown(&key); // 2.
//        if (keySignal != registeredKeyEventSignals.end())
 //           keySignal->second->OnKeyDown(key); // 3.
        break;
    case KeyEvent::KeyReleased:
        if (!IsKeyDownImmediate(key.keyCode))
            break; // If we've received a keydown for a key we haven't gotten a corresponding press for before, ignore this event.

        emit KeyEventReceived(&key); // 1.
        emit KeyReleased(&key); // 2.
        if (keySignal != registeredKeyEventSignals.end())
            keySignal->second->OnKeyReleased(key); // 3.
        break;
    default:
        assert(false);
        break;
    }

    // Update the buffered API.
    KeyPressInformation info;
    info.keyState = key.eventType;
    info.keyPressCount = key.keyPressCount;
//    info.firstPressTime = key.firstPressTime;
    newKeyEvents[key.keyCode] = info;

    // Now if this layer is registered to suppress this keypress from being processed further,
    // mark it handled.
    if (suppressedKeys.find(key.keyCode) != suppressedKeys.end())
        key.handled = true;
}

void InputContext::TriggerKeyReleaseEvent(Qt::Key keyCode)
{
    HeldKeysMap::iterator iter = heldKeysBuffered.find(keyCode);
    // If this key has not even been pressed down in this context, ignore it.
    if (iter == heldKeysBuffered.end())
        return;

    KeyEvent release;
    release.keyCode = keyCode;
    release.keyPressCount = iter->second.keyPressCount;
    release.eventType = KeyEvent::KeyReleased;
    TriggerKeyEvent(release);
}

void InputContext::TriggerMouseEvent(MouseEvent &mouse)
{
    emit MouseEventReceived(&mouse);

    switch(mouse.eventType)
    {
    case MouseEvent::MouseMove: emit MouseMove(&mouse); break;
    case MouseEvent::MouseScroll: emit MouseScroll(&mouse); break;
    case MouseEvent::MousePressed:
        switch(mouse.button)
        {
        case MouseEvent::LeftButton: emit MouseLeftPressed(&mouse); break;
        case MouseEvent::RightButton: emit MouseRightPressed(&mouse); break;
        case MouseEvent::MiddleButton: emit MouseMiddlePressed(&mouse); break;
            ///\todo XButton1 and XButton2 support?
        }
        break;
    case MouseEvent::MouseReleased: 
        switch(mouse.button)
        {
        case MouseEvent::LeftButton: emit MouseLeftReleased(&mouse); break;
        case MouseEvent::RightButton: emit MouseRightReleased(&mouse); break;
        case MouseEvent::MiddleButton: emit MouseMiddleReleased(&mouse); break;
            ///\todo XButton1 and XButton2 support?
        }
        break;
    case MouseEvent::MouseDoubleClicked: emit MouseDoubleClicked(&mouse); break;
    default:
        assert(false);
        break;
    }
}

void InputContext::TriggerGestureEvent(GestureEvent &gesture)
{
    emit GestureEventReceived(&gesture);

    switch(gesture.eventType)
    {
    case GestureEvent::GestureStarted:
        emit GestureStarted(&gesture);
        break;
    case GestureEvent::GestureUpdated:
        emit GestureUpdated(&gesture);
        break;
    case GestureEvent::GestureFinished:
    case GestureEvent::GestureCanceled:
        emit GestureFinished(&gesture);
        break;
    }    
}

void InputContext::SetPriority(int newPriority)
{
    assert(inputApi);
    inputApi->SetPriority(shared_from_this(), newPriority);
}

int InputContext::KeyPressedCount(Qt::Key keyCode) const
{
    HeldKeysMap::const_iterator iter = heldKeysBuffered.find(keyCode);
    if (iter == heldKeysBuffered.end())
        return 0; // The key is not being held down.

    if (iter->second.keyState != KeyEvent::KeyPressed && 
        iter->second.keyState != KeyEvent::KeyDown)
        return 0;

    return iter->second.keyPressCount;
}

bool InputContext::IsKeyDown(Qt::Key keyCode) const
{
    HeldKeysMap::const_iterator iter = heldKeysBuffered.find(keyCode);
    if (iter == heldKeysBuffered.end())
        return false;

    return iter->second.keyState == KeyEvent::KeyPressed || 
        iter->second.keyState == KeyEvent::KeyDown;
}

bool InputContext::IsKeyReleased(Qt::Key keyCode) const
{
    HeldKeysMap::const_iterator iter = heldKeysBuffered.find(keyCode);
    if (iter == heldKeysBuffered.end())
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

void InputContext::UpdateFrame()
{
    // Update the buffered events.
    HeldKeysMap::iterator iter = heldKeysBuffered.begin();
    while(iter != heldKeysBuffered.end())
    {
        if (iter->second.keyState == KeyEvent::KeyPressed)
        {
            iter->second.keyState = KeyEvent::KeyDown;
            ++iter;
        }
        else if (iter->second.keyState == KeyEvent::KeyReleased)
          {
            HeldKeysMap::iterator next = iter;
            ++next;
            heldKeysBuffered.erase(iter);
            iter = next;
          }
        else 
            ++iter;
    }

    // Put all new events to the buffer.
    for(HeldKeysMap::iterator iter = newKeyEvents.begin(); iter != newKeyEvents.end(); ++iter)
        heldKeysBuffered[iter->first] = iter->second;

    newKeyEvents.clear();
}

void InputContext::ReleaseAllKeys()
{
    std::vector<Qt::Key> keysToRelease;

    // We double buffer the to-be-released keys first to a temporary list, since invoking the trigger
    // function will add new entries to newKeyEvents.

    for(HeldKeysMap::iterator iter = heldKeysBuffered.begin(); iter != heldKeysBuffered.end(); ++iter)
        keysToRelease.push_back(iter->first);

    for(HeldKeysMap::iterator iter = newKeyEvents.begin(); iter != newKeyEvents.end(); ++iter)
        keysToRelease.push_back(iter->first);

    for(std::vector<Qt::Key>::iterator iter = keysToRelease.begin(); iter != keysToRelease.end(); ++iter)
        TriggerKeyReleaseEvent(*iter);
}

void InputContext::SetMouseCursorOverride(QCursor cursor)
{
    if (!mouseCursorOverride)
        mouseCursorOverride = MAKE_SHARED(QCursor, cursor);
    else
        *mouseCursorOverride = cursor;

    inputApi->ApplyMouseCursorOverride();
}

QCursor *InputContext::MouseCursorOverride() const
{
    return mouseCursorOverride.get();
}

void InputContext::ClearMouseCursorOverride()
{
    if (mouseCursorOverride)
    {
        mouseCursorOverride.reset();
        inputApi->ApplyMouseCursorOverride();
    }
}

bool InputContext::IsKeyDownImmediate(Qt::Key keyCode) const
{
    HeldKeysMap::const_iterator iter = newKeyEvents.find(keyCode);
    if (iter != newKeyEvents.end())
        return iter->second.keyState == KeyEvent::KeyDown ||
            iter->second.keyState == KeyEvent::KeyPressed;

    iter = heldKeysBuffered.find(keyCode);
    if (iter != heldKeysBuffered.end())
        return iter->second.keyState == KeyEvent::KeyDown ||
            iter->second.keyState == KeyEvent::KeyPressed;

    return false;
}
