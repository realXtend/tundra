#include "StableHeaders.h"
#include "RexMovementInput.h"
#include "InputEvents.h"
#include "RexTypes.h"
#include "InputServiceInterface.h"
#include "EventManager.h"

namespace RexLogic
{

RexMovementInput::RexMovementInput(Foundation::Framework *framework_)
{
    framework = framework_;

    // Create a new input context that this object will use to fetch the avatar and camera input from.
    input = framework->Input().RegisterInputContext("RexAvatarInput", 100);

    // Listen on both key and mouse input signals.
    connect(input.get(), SIGNAL(OnKeyEvent(KeyEvent &)), this, SLOT(HandleKeyEvent(KeyEvent &)));
    connect(input.get(), SIGNAL(OnMouseEvent(MouseEvent &)), this, SLOT(HandleMouseEvent(MouseEvent &)));
}

// Either sends an input event press or release, depending on the key event type.
void SendPressOrRelease(Foundation::EventManagerPtr eventMgr, const KeyEvent &key, int eventID)
{
    // In the AvatarControllable and CameraControllable framework, each input event id that denotes a press
    // also has a corresponding release event, which has an ID one larger than the press event.
    if (key.eventType == KeyEvent::KeyPressed)
        eventMgr->SendEvent("Input", eventID, 0);
    else if (key.eventType == KeyEvent::KeyReleased)
        eventMgr->SendEvent("Input", eventID+1, 0);
}

void RexMovementInput::HandleKeyEvent(KeyEvent &key)
{
    ///\todo Read these through input mappings configuration list.

    const Qt::Key walkForward = Qt::Key_W;
    const Qt::Key walkBackward = Qt::Key_S;
    const Qt::Key walkForward2 = Qt::Key_Up;
    const Qt::Key walkBackward2 = Qt::Key_Down;
    const Qt::Key strafeLeft = Qt::Key_A;
    const Qt::Key strafeRight = Qt::Key_D;
    const Qt::Key rotateLeft = Qt::Key_Left;
    const Qt::Key rotateRight = Qt::Key_Right;
    const Qt::Key up = Qt::Key_Space; // Jump or fly up, depending on whether in fly mode or walk mode.
    const Qt::Key down = Qt::Key_Control; // Crouch or fly down, depending on whether in fly mode or walk mode.
    const Qt::Key flyModeToggle = Qt::Key_F;
    const Qt::Key cameraModeToggle = Qt::Key_Tab;

    Foundation::EventManagerPtr eventMgr = framework->GetEventManager();

    // The following code defines the keyboard actions that are available for the avatar/freelookcamera system.
    switch(key.keyCode)
    {
    case walkForward: case walkForward2:   SendPressOrRelease(eventMgr, key, Input::Events::MOVE_FORWARD_PRESSED); break;
    case walkBackward: case walkBackward2: SendPressOrRelease(eventMgr, key, Input::Events::MOVE_BACK_PRESSED); break;
    case strafeLeft:  SendPressOrRelease(eventMgr, key, Input::Events::MOVE_LEFT_PRESSED); break;
    case strafeRight: SendPressOrRelease(eventMgr, key, Input::Events::MOVE_RIGHT_PRESSED); break;
    case rotateLeft:  SendPressOrRelease(eventMgr, key, Input::Events::ROTATE_LEFT_PRESSED); break;
    case rotateRight: SendPressOrRelease(eventMgr, key, Input::Events::ROTATE_RIGHT_PRESSED); break;
    case up:   SendPressOrRelease(eventMgr, key, Input::Events::MOVE_UP_PRESSED); break;
    case down: SendPressOrRelease(eventMgr, key, Input::Events::MOVE_DOWN_PRESSED); break;
    case flyModeToggle: 
        if (key.eventType == KeyEvent::KeyPressed)
            SendPressOrRelease(eventMgr, key, Input::Events::TOGGLE_FLYMODE); 
        break;
    case cameraModeToggle:
        if (key.eventType == KeyEvent::KeyPressed)
        {
            key.handled = true; // Suppress Qt from getting a tab next item.
            input->ReleaseAllKeys();
            eventMgr->SendEvent("Input", Input::Events::SWITCH_CAMERA_STATE, 0); 
        }
        break;
    }
}

void RexMovementInput::HandleMouseEvent(MouseEvent &mouse)
{
    Foundation::EventManagerPtr eventMgr = framework->GetEventManager();

    Input::Events::Movement movement;
    movement.x_.abs_ = mouse.x;
    movement.y_.abs_ = mouse.y;
    movement.z_.abs_ = mouse.z;
    movement.x_.rel_ = mouse.relativeX;
    movement.y_.abs_ = mouse.relativeY;
    movement.z_.abs_ = mouse.relativeZ;

    switch(mouse.eventType)
    {
    case MouseEvent::MousePressed:
        if (mouse.button == MouseEvent::LeftButton)
            eventMgr->SendEvent("Input", Input::Events::INWORLD_CLICK, &movement);
        break;
    case MouseEvent::MouseMove:
        if (mouse.IsRightButtonDown())
            eventMgr->SendEvent("Input", Input::Events::MOUSELOOK, &movement);
        else
            eventMgr->SendEvent("Input", Input::Events::MOUSEMOVE, &movement);
        break;
    case MouseEvent::MouseScroll:
    {
        Input::Events::SingleAxisMovement singleAxis;
        singleAxis.z_.screen_ = 0;
        singleAxis.z_.abs_ = 0;
        singleAxis.z_.rel_ = mouse.relativeZ;
        eventMgr->SendEvent("Input", Input::Events::SCROLL, &singleAxis);

        mouse.handled = true; // Mark this event as handled. Suppresses Qt from getting it.
        break;
    }
    }
}


} // ~RexLogic