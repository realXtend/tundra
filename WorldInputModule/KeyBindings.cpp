
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputEvents.h"
#include "KeyBindings.h"
#include <Qt>

namespace Input
{
    KeyBinding::KeyBinding ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::Key_Tab),         Input::Events::SWITCH_CAMERA_STATE));
		map.insert (make_pair (QKeySequence (Qt::Key_Period),      Input::Events::PY_RUN_COMMAND));
		map.insert (make_pair (QKeySequence (Qt::Key_Backspace),   Input::Events::PY_RESTART));
		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Plus),        Input::Events::ZOOM_IN));
		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Minus),       Input::Events::ZOOM_OUT));
        //map.insert (make_pair (QKeySequence (Qt::Key_GRAVE),     Input::Events::SHOW_DEBUG_CONSOLE));
    }

    FirstPersonBindings::FirstPersonBindings ()
    {
        using std::make_pair;
        map.insert (make_pair (QKeySequence (Qt::Key_W),           Input::Events::MOVE_FORWARD_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_S),           Input::Events::MOVE_BACK_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_A),           Input::Events::MOVE_LEFT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_D),           Input::Events::MOVE_RIGHT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Up),          Input::Events::MOVE_FORWARD_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Down),        Input::Events::MOVE_BACK_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Space),       Input::Events::MOVE_UP_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_C),           Input::Events::MOVE_DOWN_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),      Input::Events::MOVE_UP_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),    Input::Events::MOVE_DOWN_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Left),        Input::Events::MOVE_LEFT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Right),       Input::Events::MOVE_RIGHT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_F),           Input::Events::TOGGLE_FLYMODE));
    }

    ThirdPersonBindings::ThirdPersonBindings ()
    {
        using std::make_pair;
        map.insert (make_pair (QKeySequence (Qt::Key_W),           Input::Events::MOVE_FORWARD_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_S),           Input::Events::MOVE_BACK_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_A),           Input::Events::MOVE_LEFT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_D),           Input::Events::MOVE_RIGHT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Up),          Input::Events::MOVE_FORWARD_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Down),        Input::Events::MOVE_BACK_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Space),       Input::Events::MOVE_UP_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_C),           Input::Events::MOVE_DOWN_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),      Input::Events::MOVE_UP_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),    Input::Events::MOVE_DOWN_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Left),        Input::Events::ROTATE_LEFT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Right),       Input::Events::ROTATE_RIGHT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_F),           Input::Events::TOGGLE_FLYMODE));
    }

    FreeCameraBindings::FreeCameraBindings ()
    {
        using std::make_pair;
        map.insert (make_pair (QKeySequence (Qt::Key_W),           Input::Events::MOVE_FORWARD_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_S),           Input::Events::MOVE_BACK_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_A),           Input::Events::MOVE_LEFT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_D),           Input::Events::MOVE_RIGHT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Up),          Input::Events::MOVE_FORWARD_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Down),        Input::Events::MOVE_BACK_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Space),       Input::Events::MOVE_UP_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_C),           Input::Events::MOVE_DOWN_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),      Input::Events::MOVE_UP_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),    Input::Events::MOVE_DOWN_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Left),        Input::Events::ROTATE_LEFT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_Right),       Input::Events::ROTATE_RIGHT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_J),           Input::Events::ROTATE_LEFT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_L),           Input::Events::ROTATE_RIGHT_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_I),           Input::Events::ROTATE_UP_PRESSED));
        map.insert (make_pair (QKeySequence (Qt::Key_K),           Input::Events::ROTATE_DOWN_PRESSED));
    }
}

