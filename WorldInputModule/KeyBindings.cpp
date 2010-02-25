
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

        map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Tab),   
                    make_pair (Input::Events::SWITCH_CAMERA_STATE, 0)));

        map.insert (make_pair (QKeySequence (Qt::ALT + Qt::Key_Period),           
                    make_pair (Input::Events::PY_RUN_COMMAND, 0)));

		map.insert (make_pair (QKeySequence (Qt::Key_F11), //to have the same in login screen hack as well. Backspace),        
                    make_pair (Input::Events::PY_RESTART, 0)));

        map.insert (make_pair (QKeySequence (Qt::Key_F1),
                    make_pair (Input::Events::SHOW_DEBUG_CONSOLE, 0)));

		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_D), 
					make_pair (Input::Events::PY_DUPLICATE_DRAG, 0)));
		
		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_M), 
					make_pair (Input::Events::PY_OBJECTEDIT_TOGGLE_MOVE, 0)));

		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_S), 
				make_pair (Input::Events::PY_OBJECTEDIT_TOGGLE_SCALE, 0)));

		//map.insert (make_pair (QKeySequence (Qt::ALT + Qt::Key_R), 
					//make_pair (Input::Events::PY_OBJECTEDIT_TOGGLE_ROTATE, 0)));

		map.insert (make_pair (QKeySequence (Qt::Key_Delete), // XXX, alt+del viable?
				make_pair (Input::Events::NAALI_DELETE, 0)));

		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Z), 
				make_pair (Input::Events::NAALI_UNDO, 0)));
    }

    FirstPersonBindings::FirstPersonBindings ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::Key_F), 
                    make_pair (Input::Events::TOGGLE_FLYMODE, 0)));

		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Plus),  
                    make_pair (Input::Events::ZOOM_IN_PRESSED, Input::Events::ZOOM_IN_RELEASED)));

		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Minus), 
                    make_pair (Input::Events::ZOOM_OUT_PRESSED, Input::Events::ZOOM_OUT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_W),                
                    make_pair (Input::Events::MOVE_FORWARD_PRESSED, Input::Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_S),                
                    make_pair (Input::Events::MOVE_BACK_PRESSED, Input::Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_A),                
                    make_pair (Input::Events::MOVE_LEFT_PRESSED, Input::Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_D),                
                    make_pair (Input::Events::MOVE_RIGHT_PRESSED, Input::Events::MOVE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Up),               
                    make_pair (Input::Events::MOVE_FORWARD_PRESSED, Input::Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Down),             
                    make_pair (Input::Events::MOVE_BACK_PRESSED, Input::Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Space),            
                    make_pair (Input::Events::MOVE_UP_PRESSED, Input::Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_C),                
                    make_pair (Input::Events::MOVE_DOWN_PRESSED, Input::Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),           
                    make_pair (Input::Events::MOVE_UP_PRESSED, Input::Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),         
                    make_pair (Input::Events::MOVE_DOWN_PRESSED, Input::Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Left),             
                    make_pair (Input::Events::MOVE_LEFT_PRESSED, Input::Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Right),            
                    make_pair (Input::Events::MOVE_RIGHT_PRESSED, Input::Events::MOVE_RIGHT_RELEASED)));
    }

    ThirdPersonBindings::ThirdPersonBindings ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::Key_F),                
                    make_pair (Input::Events::TOGGLE_FLYMODE, 0)));

		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Plus),  
                    make_pair (Input::Events::ZOOM_IN_PRESSED, Input::Events::ZOOM_IN_RELEASED)));

		map.insert (make_pair (QKeySequence (Qt::CTRL + Qt::Key_Minus), 
                    make_pair (Input::Events::ZOOM_OUT_PRESSED, Input::Events::ZOOM_OUT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_W),                
                    make_pair (Input::Events::MOVE_FORWARD_PRESSED, Input::Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_S),                
                    make_pair (Input::Events::MOVE_BACK_PRESSED, Input::Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_A),                
                    make_pair (Input::Events::MOVE_LEFT_PRESSED, Input::Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_D),                
                    make_pair (Input::Events::MOVE_RIGHT_PRESSED, Input::Events::MOVE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Up),               
                    make_pair (Input::Events::MOVE_FORWARD_PRESSED, Input::Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Down),             
                    make_pair (Input::Events::MOVE_BACK_PRESSED, Input::Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Space),            
                    make_pair (Input::Events::MOVE_UP_PRESSED, Input::Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_C),                
                    make_pair (Input::Events::MOVE_DOWN_PRESSED, Input::Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),           
                    make_pair (Input::Events::MOVE_UP_PRESSED, Input::Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),         
                    make_pair (Input::Events::MOVE_DOWN_PRESSED, Input::Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Left),             
                    make_pair (Input::Events::ROTATE_LEFT_PRESSED, Input::Events::ROTATE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Right),            
                    make_pair (Input::Events::ROTATE_RIGHT_PRESSED, Input::Events::ROTATE_RIGHT_RELEASED)));
    }

    FreeCameraBindings::FreeCameraBindings ()
    {
        using std::make_pair;

        map.insert (make_pair (QKeySequence (Qt::Key_W),                
                    make_pair (Input::Events::MOVE_FORWARD_PRESSED, Input::Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_S),                
                    make_pair (Input::Events::MOVE_BACK_PRESSED, Input::Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_A),                
                    make_pair (Input::Events::MOVE_LEFT_PRESSED, Input::Events::MOVE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_D),                
                    make_pair (Input::Events::MOVE_RIGHT_PRESSED, Input::Events::MOVE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Up),               
                    make_pair (Input::Events::MOVE_FORWARD_PRESSED, Input::Events::MOVE_FORWARD_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Down),             
                    make_pair (Input::Events::MOVE_BACK_PRESSED, Input::Events::MOVE_BACK_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Space),            
                    make_pair (Input::Events::MOVE_UP_PRESSED, Input::Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_C),                
                    make_pair (Input::Events::MOVE_DOWN_PRESSED, Input::Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageUp),           
                    make_pair (Input::Events::MOVE_UP_PRESSED, Input::Events::MOVE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_PageDown),         
                    make_pair (Input::Events::MOVE_DOWN_PRESSED, Input::Events::MOVE_DOWN_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Left),             
                    make_pair (Input::Events::ROTATE_LEFT_PRESSED, Input::Events::ROTATE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_Right),            
                    make_pair (Input::Events::ROTATE_RIGHT_PRESSED, Input::Events::ROTATE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_J),                
                    make_pair (Input::Events::ROTATE_LEFT_PRESSED, Input::Events::ROTATE_LEFT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_L),                
                    make_pair (Input::Events::ROTATE_RIGHT_PRESSED, Input::Events::ROTATE_RIGHT_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_I),                
                    make_pair (Input::Events::ROTATE_UP_PRESSED, Input::Events::ROTATE_UP_RELEASED)));

        map.insert (make_pair (QKeySequence (Qt::Key_K),                
                    make_pair (Input::Events::ROTATE_DOWN_PRESSED, Input::Events::ROTATE_DOWN_RELEASED)));
    }
}

