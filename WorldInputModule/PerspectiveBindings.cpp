// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PerspectiveBindings.h"
#include "InputEvents.h"

namespace Input
{
    KeyBinding::KeyBinding ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Tab),
                    make_pair (Events::SWITCH_CAMERA_STATE, 0)));

		map.insert (make_pair (QKeySequence (Qt::Key_T),
                    make_pair (Events::CAMERA_TRIPOD, 0)));

        map.insert (make_pair (QKeySequence (Qt::ALT + Qt::Key_Period),
                    make_pair (Events::PY_RUN_COMMAND, 0)));

        map.insert (make_pair (QKeySequence (Qt::Key_F11), //to have the same in login screen hack as well. Backspace)
                    make_pair (Events::PY_RESTART, 0)));

        map.insert (make_pair (QKeySequence (Qt::Key_F1),
                    make_pair (Events::SHOW_DEBUG_CONSOLE, 0)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_D),
                    make_pair (Events::PY_DUPLICATE_DRAG, 0)));
        
        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_M),
                    make_pair (Events::PY_OBJECTEDIT_TOGGLE_MOVE, 0)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_S), 
                    make_pair (Events::PY_OBJECTEDIT_TOGGLE_SCALE, 0)));

        //map.insert (make_pair (QKeySequence (Qt::ALT + Qt::Key_R), 
                    //make_pair (Events::PY_OBJECTEDIT_TOGGLE_ROTATE, 0)));

        map.insert (make_pair (QKeySequence (Qt::Key_Delete), // XXX, alt+del viable?
                make_pair (Events::NAALI_DELETE, 0)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Z),
                make_pair (Events::NAALI_UNDO, 0)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_L),
                make_pair (Events::NAALI_OBJECTLINK, 0)));

        map.insert (make_pair (QKeySequence (Qt::ALT + Qt::CTRL + Qt::Key_L),
                make_pair (Events::NAALI_OBJECTUNLINK, 0)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Space),
                make_pair (Events::UNFOCUS_UI, 0)));
		/* tofilovski focus on object */
		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_R),
			make_pair (Events::FOCUS_ON_OBJECT, 0)));
    }

    FirstPersonBindings::FirstPersonBindings ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::Key_F),
                    make_pair (Events::TOGGLE_FLYMODE, 0)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Plus),
                    make_pair (Events::ZOOM_IN_PRESSED, Events::ZOOM_IN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Minus),
                    make_pair (Events::ZOOM_OUT_PRESSED, Events::ZOOM_OUT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_W),
                    make_pair (Events::MOVE_FORWARD_PRESSED, Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_S),
                    make_pair (Events::MOVE_BACK_PRESSED, Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_A),
                    make_pair (Events::MOVE_LEFT_PRESSED, Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_D),
                    make_pair (Events::MOVE_RIGHT_PRESSED, Events::MOVE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Up),
                    make_pair (Events::MOVE_FORWARD_PRESSED, Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Down),
                    make_pair (Events::MOVE_BACK_PRESSED, Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Space),
                    make_pair (Events::MOVE_UP_PRESSED, Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_C),
                    make_pair (Events::MOVE_DOWN_PRESSED, Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),
                    make_pair (Events::MOVE_UP_PRESSED, Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),
                    make_pair (Events::MOVE_DOWN_PRESSED, Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Left),
                    make_pair (Events::MOVE_LEFT_PRESSED, Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Right),
                    make_pair (Events::MOVE_RIGHT_PRESSED, Events::MOVE_RIGHT_RELEASED)));
		/* tofilovski focus on object */
		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_R),
                    make_pair (Events::FOCUS_ON_OBJECT, 0)));
    }

    ThirdPersonBindings::ThirdPersonBindings ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::Key_F),
                    make_pair (Events::TOGGLE_FLYMODE, 0)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Plus),
                    make_pair (Events::ZOOM_IN_PRESSED, Events::ZOOM_IN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Minus),
                    make_pair (Events::ZOOM_OUT_PRESSED, Events::ZOOM_OUT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_W),
                    make_pair (Events::MOVE_FORWARD_PRESSED, Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_S),
                    make_pair (Events::MOVE_BACK_PRESSED, Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_A),
                    make_pair (Events::MOVE_LEFT_PRESSED, Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_D),
                    make_pair (Events::MOVE_RIGHT_PRESSED, Events::MOVE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Up),
                    make_pair (Events::MOVE_FORWARD_PRESSED, Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Down),
                    make_pair (Events::MOVE_BACK_PRESSED, Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Space),
                    make_pair (Events::MOVE_UP_PRESSED, Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_C),
                    make_pair (Events::MOVE_DOWN_PRESSED, Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),
                    make_pair (Events::MOVE_UP_PRESSED, Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),
                    make_pair (Events::MOVE_DOWN_PRESSED, Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Left),
                    make_pair (Events::ROTATE_LEFT_PRESSED, Events::ROTATE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Right),
                    make_pair (Events::ROTATE_RIGHT_PRESSED, Events::ROTATE_RIGHT_RELEASED)));

		/* tofilovski focus on object */
		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_R),
                    make_pair (Events::FOCUS_ON_OBJECT, 0)));
    }

    FreeCameraBindings::FreeCameraBindings ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::Key_W),
                    make_pair (Events::MOVE_FORWARD_PRESSED, Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_S),
                    make_pair (Events::MOVE_BACK_PRESSED, Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_A),
                    make_pair (Events::MOVE_LEFT_PRESSED, Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_D),
                    make_pair (Events::MOVE_RIGHT_PRESSED, Events::MOVE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Up),
                    make_pair (Events::MOVE_FORWARD_PRESSED, Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Down),
                    make_pair (Events::MOVE_BACK_PRESSED, Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Space),
                    make_pair (Events::MOVE_UP_PRESSED, Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_C),
                    make_pair (Events::MOVE_DOWN_PRESSED, Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),
                    make_pair (Events::MOVE_UP_PRESSED, Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),
                    make_pair (Events::MOVE_DOWN_PRESSED, Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Left),
                    make_pair (Events::ROTATE_LEFT_PRESSED, Events::ROTATE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Right),
                    make_pair (Events::ROTATE_RIGHT_PRESSED, Events::ROTATE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_J),
                    make_pair (Events::ROTATE_LEFT_PRESSED, Events::ROTATE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_L),
                    make_pair (Events::ROTATE_RIGHT_PRESSED, Events::ROTATE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_I),
                    make_pair (Events::ROTATE_UP_PRESSED, Events::ROTATE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_K),
                    make_pair (Events::ROTATE_DOWN_PRESSED, Events::ROTATE_DOWN_RELEASED)));
    }
}

