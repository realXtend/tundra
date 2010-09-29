// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexMovementInput.h"
#include "InputEvents.h"
#include "RexTypes.h"
#include "Input.h"
#include "EventManager.h"

#include <QApplication>

namespace RexLogic
{

RexMovementInput::RexMovementInput(Foundation::Framework *framework_)
{
    framework = framework_;

    // Create a new input context that this object will use to fetch the avatar and camera input from.
    input = framework->GetInput()->RegisterInputContext("RexAvatarInput", 100);

    // To be sure that Qt doesn't play tricks on us and miss a mouse release when we're in FPS mode,
    // grab the mouse movement input over Qt.
    input->SetTakeMouseEventsOverQt(true);

    // Listen on both key and mouse input signals.
    connect(input.get(), SIGNAL(OnKeyEvent(KeyEvent *)), this, SLOT(HandleKeyEvent(KeyEvent *)));
    connect(input.get(), SIGNAL(OnMouseEvent(MouseEvent *)), this, SLOT(HandleMouseEvent(MouseEvent *)));
}

// Either sends an input event press or release, depending on the key event type.
void SendPressOrRelease(Foundation::EventManagerPtr eventMgr, const KeyEvent *key, int eventID)
{
    // In the AvatarControllable and CameraControllable framework, each input event id that denotes a press
    // also has a corresponding release event, which has an ID one larger than the press event.
    if (key->eventType == KeyEvent::KeyPressed)
        eventMgr->SendEvent("Input", eventID, 0);
    else if (key->eventType == KeyEvent::KeyReleased)
        eventMgr->SendEvent("Input", eventID+1, 0);
}

void RexMovementInput::HandleKeyEvent(KeyEvent *key)
{
    // This function handles received input events and translates them to the "traditional"-style
    // Naali input events. New modules should really prefer using an InputContext of their
    // own to read input data, or use the QtInputService API directly.

    Input *inputService = framework->GetInput();

    Foundation::EventManagerPtr eventMgr = framework->GetEventManager();

    ///\todo Accessing the key bindings by string names is a bit silly, since for each key event, this
    /// generates a string compare. Now for every key, we do this for every key we want to act on.
    /// For optimization we can prehash the strings, but that will occur later when we profile performance
    /// issues here.
    PROFILE(RexMovement_HandleKeyEvent);

    ///\bug Qt does not have a separate key code for keypad minus, but instead uses "Qt::Key_Minus | Qt::KeypadModifier". However,
    /// this causes it to fail when showing it in portable text mode in the key bindings editor, i.e. 
    /// QKeySequence(Qt::Key_Minus | Qt::KeypadModifier | Qt::ControlModifier).toString(QKeySequence::NativeText); fails.

    // As a workaround, the following two keys are hardcoded to keypad plus. See http://bugreports.qt.nokia.com/browse/QTBUG-2913
    // and http://qt.nokia.com/developer/task-tracker/index_html?method=entry&id=229868

    const QKeySequence zoomIn = inputService->KeyBinding("Avatar.ZoomIn", Qt::Key_Minus | Qt::ControlModifier);
    const QKeySequence zoomOut = inputService->KeyBinding("Avatar.ZoomOut", Qt::Key_Plus | Qt::ControlModifier);

    // For the zoom in and out keys, key repeats trigger continuous actions.
    if ((key->KeyWithModifier() & ~Qt::KeypadModifier) == zoomIn) SendPressOrRelease(eventMgr, key, InputEvents::ZOOM_IN_PRESSED);
    if ((key->KeyWithModifier() & ~Qt::KeypadModifier) == zoomOut) SendPressOrRelease(eventMgr, key, InputEvents::ZOOM_OUT_PRESSED);

    // For the following keys, we ignore all key presses that are repeats.
    if (key->eventType == KeyEvent::KeyPressed && key->keyPressCount > 1)
        return;

    const QKeySequence walkForward =   inputService->KeyBinding("Avatar.WalkForward", Qt::Key_W);
    const QKeySequence walkBackward =  inputService->KeyBinding("Avatar.WalkBack", Qt::Key_S);
    const QKeySequence walkForward2 =  inputService->KeyBinding("Avatar.WalkForward2", Qt::Key_Up);
    const QKeySequence walkBackward2 = inputService->KeyBinding("Avatar.WalkBack2", Qt::Key_Down);
    const QKeySequence strafeLeft =  inputService->KeyBinding("Avatar.StrafeLeft", Qt::Key_A);
    const QKeySequence strafeRight = inputService->KeyBinding("Avatar.StrafeRight", Qt::Key_D);
    const QKeySequence rotateLeft =  inputService->KeyBinding("Avatar.RotateLeft", Qt::Key_Left);
    const QKeySequence rotateRight = inputService->KeyBinding("Avatar.RotateRight", Qt::Key_Right);
    const QKeySequence up =   inputService->KeyBinding("Avatar.Up", Qt::Key_Space); // Jump or fly up, depending on whether in fly mode or walk mode.
    const QKeySequence up2 =   inputService->KeyBinding("Avatar.Up2", Qt::Key_PageUp); // Jump or fly up, depending on whether in fly mode or walk mode.
    const QKeySequence down = inputService->KeyBinding("Avatar.Down", /*Qt::Key_Control*/Qt::Key_C); // Crouch or fly down, depending on whether in fly mode or walk mode.
    const QKeySequence down2 = inputService->KeyBinding("Avatar.Down2", /*Qt::Key_Control*/Qt::Key_PageDown); // Crouch or fly down, depending on whether in fly mode or walk mode.
    const QKeySequence flyModeToggle =    inputService->KeyBinding("Avatar.ToggleFly", Qt::Key_F);
    const QKeySequence cameraModeToggle = inputService->KeyBinding("Avatar.ToggleCameraMode", Qt::Key_Tab);
    const QKeySequence cameraModeTripod = inputService->KeyBinding("Avatar.TripodCameraMode", Qt::Key_T);

    // The following code defines the keyboard actions that are available for the avatar/freelookcamera system.
    if (key->keyCode == walkForward || key->keyCode == walkForward2) SendPressOrRelease(eventMgr, key, InputEvents::MOVE_FORWARD_PRESSED);
    if (key->keyCode == walkBackward || key->keyCode == walkBackward2) SendPressOrRelease(eventMgr, key, InputEvents::MOVE_BACK_PRESSED);
    if (key->keyCode == strafeLeft) SendPressOrRelease(eventMgr, key, InputEvents::MOVE_LEFT_PRESSED);
    if (key->keyCode == strafeRight) SendPressOrRelease(eventMgr, key, InputEvents::MOVE_RIGHT_PRESSED); 
    if (key->keyCode == rotateLeft)  SendPressOrRelease(eventMgr, key, InputEvents::ROTATE_LEFT_PRESSED); 
    if (key->keyCode == rotateRight) SendPressOrRelease(eventMgr, key, InputEvents::ROTATE_RIGHT_PRESSED); 
    if (key->keyCode == up || key->keyCode == up2)   SendPressOrRelease(eventMgr, key, InputEvents::MOVE_UP_PRESSED); 
    if (key->keyCode == down || key->keyCode == down2) SendPressOrRelease(eventMgr, key, InputEvents::MOVE_DOWN_PRESSED); 
    if (key->keyCode == flyModeToggle) SendPressOrRelease(eventMgr, key, InputEvents::TOGGLE_FLYMODE); 
    if (key->keyCode == cameraModeToggle && key->eventType == KeyEvent::KeyPressed)
    {
        if (key->keyCode == Qt::Key_Tab)
            key->handled = true; // Suppress Qt from moving the focus to a widget.
        input->ReleaseAllKeys();
        eventMgr->SendEvent("Input", InputEvents::SWITCH_CAMERA_STATE, 0); 
    }
    if (key->keyCode == cameraModeTripod && key->eventType == KeyEvent::KeyPressed)
    {
        if (key->keyCode == Qt::Key_T)
            key->handled = true; // Suppress Qt from moving the focus to a widget.
        input->ReleaseAllKeys();
		eventMgr->SendEvent("Input", InputEvents::CAMERA_TRIPOD, 0); 
    }
    if (key->keyCode == Qt::Key_Alt && key->eventType == KeyEvent::KeyReleased)
    {
        eventMgr->SendEvent("Input", InputEvents::ALT_LEFTCLICK_REL, 0);
    }
}

void RexMovementInput::HandleMouseEvent(MouseEvent *mouse)
{
    Foundation::EventManagerPtr eventMgr = framework->GetEventManager();

    // This function handles received input events and translates them to the "traditional"-style
    // Naali input events. New modules should really prefer using an InputContext of their
    // own to read input data, or use the QtInputService API directly.

    // We pass this event struct forward to Naali event tree in most cases above,
    // so fill it here already.
    InputEvents::Movement movement;
    movement.x_.abs_ = mouse->x;
    movement.y_.abs_ = mouse->y;
    movement.z_.abs_ = mouse->z;
    movement.x_.rel_ = mouse->relativeX;
    movement.y_.rel_ = mouse->relativeY;
    movement.z_.rel_ = mouse->relativeZ;

    switch(mouse->eventType)
    {
    case MouseEvent::MousePressed:
        if (!mouse->itemUnderMouse)
        {
            // Left mouse button press produces click events on world objects (prims, mostly)
            if (mouse->button == MouseEvent::LeftButton)
                eventMgr->SendEvent("Input", InputEvents::INWORLD_CLICK, &movement);
      
            if(QApplication::keyboardModifiers() == Qt::AltModifier)
            {
                eventMgr->SendEvent("Input", InputEvents::ALT_LEFTCLICK, &movement);
            }

            // When we start a right mouse button drag, hide the mouse cursor to enter relative mode
            // mouse input.
            if (mouse->button == MouseEvent::RightButton)
                framework->GetInput()->SetMouseCursorVisible(false);
        }
        break;
    case MouseEvent::MouseReleased:
        // Coming out of a right mouse button drag, restore the mouse cursor to visible state.
        if (mouse->button == MouseEvent::RightButton)
            framework->GetInput()->SetMouseCursorVisible(true);
        break;
    case MouseEvent::MouseMove:
        if (mouse->IsRightButtonDown() && !framework->GetInput()->IsMouseCursorVisible()) // When RMB is down, post the Naali MOUSELOOK, which rotates the avatar/camera.
        {
           eventMgr->SendEvent("Input", InputEvents::MOUSELOOK, &movement);
           mouse->handled = true; // Mouse is in RMB mouselook mode, suppress others from getting the move event.
        }
        else if (mouse->IsLeftButtonDown())
            eventMgr->SendEvent("Input", InputEvents::MOUSEDRAG, &movement);
        break;
    case MouseEvent::MouseScroll:
    {
        InputEvents::SingleAxisMovement singleAxis;
        singleAxis.z_.screen_ = 0;
        singleAxis.z_.abs_ = 0;
        singleAxis.z_.rel_ = mouse->relativeZ;
        eventMgr->SendEvent("Input", InputEvents::SCROLL, &singleAxis);

        // Mark this event as handled. Suppresses Qt from getting it. Otherwise mouse-scrolling over an 
        // unactivated Qt widget will cause keyboard focus to go to it, which stops all other scene input.
        ///\todo Due to this, if you have a 2D webview/media url window open, you have to first left-click on it
        /// to give keyboard focus to it, after which the mouse wheel will start scrolling the webview window.
        /// Would be nice to somehow detect which windows are interested in mouse scroll events, and give them priority.
        if (!mouse->itemUnderMouse)
            mouse->handled = true; 
        break;
    }
    case MouseEvent::MouseDoubleClicked:
    {
        eventMgr->SendEvent("Input", InputEvents::MOUSE_DOUBLECLICK, &movement);
        break;
    }
    }
}

} // ~RexLogic
