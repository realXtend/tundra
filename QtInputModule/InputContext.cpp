// For conditions of distribution and use, see copyright notice in license.txt

#include "InputContext.h"

InputContext::InputContext(const char *name_)
:name(name_)
{
}

void InputContext::TriggerKeyEvent(KeyEvent &key)
{
    emit OnKeyEvent(key);

    KeyEventSignalMap::iterator iter = registeredKeyEventSignals.find(key.keyCode);

    switch(key.eventType)
    {
    case KeyEvent::KeyPressed: 
        emit KeyPressed(key); 
        if (iter != registeredKeyEventSignals.end())
            iter->second.OnKeyPressed(key);
        break;
    case KeyEvent::KeyDown:
        emit KeyDown(key); 
        if (iter != registeredKeyEventSignals.end())
            iter->second.OnKeyDown(key);
        break;
    case KeyEvent::KeyReleased:
        emit KeyReleased(key); 
        if (iter != registeredKeyEventSignals.end())
            iter->second.OnKeyReleased(key);
        break;
    default:
        assert(false);
        break;
    }
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

