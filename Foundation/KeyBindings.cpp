// For conditions of distribution and use, see copyright notice in license.txt
///\file KeyBindings.cpp
///\todo The functionality in this file is no longer used, and will be removed.

#include "StableHeaders.h"
#include "KeyBindings.h"
#include "../RexCommon/InputEvents.h"

namespace Foundation
{
    //=========================================================================
    //

    EventPair::EventPair(event_id_t enter_event_id, event_id_t leave_event_id) :
        enter_id(enter_event_id),
        leave_id(leave_event_id)
    {
    }

    EventPair::EventPair(std::pair<int,int> event_ids) :
        enter_id(event_ids.first),
        leave_id(event_ids.second)
    {
    }

    bool EventPair::operator ==(EventPair compare_pair)
    {
        if (compare_pair.enter_id == enter_id && compare_pair.leave_id == leave_id)
            return true;
        else
            return false;
    }

    //=========================================================================
    //

    Binding::Binding(QKeySequence key_sequence, EventPair event_id_pair) :
        sequence(key_sequence),
        event_ids(event_id_pair)
    {

    }

    bool Binding::operator==(Binding binding)
    {
        return binding.event_ids == event_ids;
    }

    //=========================================================================
    //

    KeyBindings::KeyBindings()
    {
        config_keys_ << "move.back" << "move.forward" << "move.up" << "move.down"  << "move.left" << "move.right"
             << "naali.delete" << "naali.undo" << "naali.object.link" << "naali.object.unlink" << "naali.toggle.ether" << "naali.toggle.worldchat"
             << "python.duplicate.drag" << "python.object.toggle.move" << "python.object.toggle.scale" << "python.restart" << "python.run"
             << "toggle.camera" << "toggle.console" << "toggle.fly" 
             << "rotate.left" << "rotate.right"
             << "zoom.in" << "zoom.out" << "naali.unfocus.ui"
			 << "toggle.tripod"
			 << "object.focus";
    }

    void KeyBindings::BindKey(Binding binding)
    {
        binding.config_key = NameForEvent(binding.event_ids.enter_id);
        bindings_.push_back(binding);
    }

    void KeyBindings::BindKey(QKeySequence sequence, std::pair<int,int> event_ids)
    {
        Binding binding(sequence, EventPair(event_ids.first, event_ids.second));
        BindKey(binding);
    }

    KeyBindingList KeyBindings::GetBindings()
    {
        return bindings_;
    }

    std::list<Binding> KeyBindings::GetBindings(EventPair event_pair)
    {
        std::list<Binding> return_list;
        
        KeyBindingList::const_iterator iter = bindings_.begin();
        KeyBindingList::const_iterator end = bindings_.end();
        while (iter != end)
        {
            Binding binding = (*iter);
            if (binding.event_ids == event_pair)
                return_list.push_back(binding);
            iter++;
        }
        return return_list;
    }

    std::list<Binding> KeyBindings::GetBindings(QString event_config_key)
    {
        std::list<Binding> return_list;
        
        KeyBindingList::const_iterator iter = bindings_.begin();
        KeyBindingList::const_iterator end = bindings_.end();
        while (iter != end)
        {
            Binding binding = (*iter);
            if (binding.config_key == event_config_key)
                return_list.push_back(binding);
            iter++;
        }
        return return_list;
    }

    std::pair<int,int> KeyBindings::GetEventPair(QKeySequence sequence)
    {
        std::pair<int,int> find_pair(0,0);
        KeyBindingList::const_iterator iter = bindings_.begin();
        KeyBindingList::const_iterator end = bindings_.end();

        while (iter != end)
        {
            Binding binding = (*iter);
            if (binding.sequence == sequence)
            {
                find_pair.first = binding.event_ids.enter_id;
                find_pair.second = binding.event_ids.leave_id;
                break;
            }
            iter++;
        }

        return find_pair;
    }

    QStringList KeyBindings::GetConfigKeys()
    {
        return config_keys_;
    }

    QString KeyBindings::NameForEvent(int event_id)
    {
        QString name;
        switch (event_id)
        {
/*
            // Fly
            case Input::Events::TOGGLE_FLYMODE:
                name = "toggle.fly";
                break;

            // Camera
            case Input::Events::SWITCH_CAMERA_STATE:
                name = "toggle.camera";
                break;

			// Lock Camera - Tripod
			case Input::Events::CAMERA_TRIPOD:
                name = "toggle.tripod";
                break;

			case Input::Events::FOCUS_ON_OBJECT:
				name = "object.focus";
				break;

            // Console
            case Input::Events::SHOW_DEBUG_CONSOLE:
                name = "toggle.console";
                break;

            // Zoom
            case Input::Events::ZOOM_IN_PRESSED:
                name = "zoom.in";
                break;
            case Input::Events::ZOOM_OUT_PRESSED:
                name = "zoom.out";
                break;

            // Move
            case Input::Events::MOVE_FORWARD_PRESSED:
                name = "move.forward";
                break;
            case Input::Events::MOVE_BACK_PRESSED:
                name = "move.back";
                break;
            case Input::Events::MOVE_LEFT_PRESSED:
                name = "move.left";
                break;
            case Input::Events::MOVE_RIGHT_PRESSED:
                name = "move.right";
                break;
            case Input::Events::MOVE_UP_PRESSED:
                name = "move.up";
                break;
            case Input::Events::MOVE_DOWN_PRESSED:
                name = "move.down";
                break;

            // Rotate
            case Input::Events::ROTATE_LEFT_PRESSED:
                name = "rotate.left";
                break;
            case Input::Events::ROTATE_RIGHT_PRESSED:
                name = "rotate.right";
                break;

            // Naali
            case Input::Events::NAALI_DELETE:
                name = "naali.delete";
                break;
            case Input::Events::NAALI_UNDO:
                name = "naali.undo";
                break;
            case Input::Events::NAALI_OBJECTLINK:
                name = "naali.object.link";
                break;
            case Input::Events::NAALI_OBJECTUNLINK:
                name = "naali.object.unlink";
                break;
            case Input::Events::NAALI_TOGGLE_ETHER:
                name = "naali.toggle.ether";
                break;
            case Input::Events::NAALI_TOGGLE_WORLDCHAT:
                name = "naali.toggle.worldchat";
                break;
            case Input::Events::UNFOCUS_UI:
                name = "naali.unfocus.ui";
                break;

            // Python
            case Input::Events::PY_RUN_COMMAND:
                name = "python.run";
                break;
            case Input::Events::PY_RESTART:
                name = "python.restart";
                break;
            case Input::Events::PY_DUPLICATE_DRAG:
                name = "python.duplicate.drag";
                break;
            case Input::Events::PY_OBJECTEDIT_TOGGLE_MOVE:
                name = "python.object.toggle.move";
                break;
            case Input::Events::PY_OBJECTEDIT_TOGGLE_SCALE:
                name = "python.object.toggle.scale";
                break;
*/
            // Unknown
            default:
                name = "";
                break;
        }
        return name;
    }

    std::pair<int,int> KeyBindings::EventPairForName(QString name)
    {
        std::pair<int,int> id_pair;
/*
        if (name == "toggle.fly")
        {
            id_pair.first = Input::Events::TOGGLE_FLYMODE;
            id_pair.second = 0;
        }
        else if (name == "toggle.camera")
        {
            id_pair.first = Input::Events::SWITCH_CAMERA_STATE;
            id_pair.second = 0;
        }
		else if (name == "toggle.tripod")
        {
			id_pair.first = Input::Events::CAMERA_TRIPOD;
            id_pair.second = 0;
        }
		else if (name == "object.focus")
        {
			id_pair.first = Input::Events::FOCUS_ON_OBJECT;
			id_pair.second = 0;
        }
        else if (name == "toggle.console")
        {
            id_pair.first = Input::Events::SHOW_DEBUG_CONSOLE;
            id_pair.second = 0;
        }
        else if (name == "zoom.in")
        {
            id_pair.first = Input::Events::ZOOM_IN_PRESSED;
            id_pair.second = Input::Events::ZOOM_IN_RELEASED;
        }
        else if (name == "zoom.out")
        {
            id_pair.first = Input::Events::ZOOM_OUT_PRESSED;
            id_pair.second = Input::Events::ZOOM_OUT_RELEASED;
        }
        else if (name == "move.forward")
        {
            id_pair.first = Input::Events::MOVE_FORWARD_PRESSED;
            id_pair.second = Input::Events::MOVE_FORWARD_RELEASED;
        }
        else if (name == "move.back")
        {
            id_pair.first = Input::Events::MOVE_BACK_PRESSED;
            id_pair.second = Input::Events::MOVE_BACK_RELEASED;
        }
        else if (name == "move.left")
        {
            id_pair.first = Input::Events::MOVE_LEFT_PRESSED;
            id_pair.second = Input::Events::MOVE_LEFT_RELEASED;
        }
        else if (name == "move.right")
        {
            id_pair.first = Input::Events::MOVE_RIGHT_PRESSED;
            id_pair.second = Input::Events::MOVE_RIGHT_RELEASED;
        }
        else if (name == "move.up")
        {
            id_pair.first = Input::Events::MOVE_UP_PRESSED;
            id_pair.second = Input::Events::MOVE_UP_RELEASED;
        }
        else if (name == "move.down")
        {
            id_pair.first = Input::Events::MOVE_DOWN_PRESSED;
            id_pair.second = Input::Events::MOVE_DOWN_RELEASED;
        }
        else if (name == "rotate.left")
        {
            id_pair.first = Input::Events::ROTATE_LEFT_PRESSED;
            id_pair.second = Input::Events::ROTATE_LEFT_RELEASED;
        }
        else if (name == "rotate.right")
        {
            id_pair.first = Input::Events::ROTATE_RIGHT_PRESSED;
            id_pair.second = Input::Events::ROTATE_RIGHT_RELEASED;
        }
        else if (name == "naali.delete")
        {
            id_pair.first = Input::Events::NAALI_DELETE;
            id_pair.second = 0;
        }
        else if (name == "naali.undo")
        {
            id_pair.first = Input::Events::NAALI_UNDO;
            id_pair.second = 0;
        }
        else if (name == "naali.object.link")
        {
            id_pair.first = Input::Events::NAALI_OBJECTLINK;
            id_pair.second = 0;
        }
        else if (name == "naali.object.unlink")
        {
            id_pair.first = Input::Events::NAALI_OBJECTUNLINK;
            id_pair.second = 0;
        }
        else if (name == "naali.toggle.ether")
        {
            id_pair.first = Input::Events::NAALI_TOGGLE_ETHER;
            id_pair.second = 0;
        }
        else if (name == "naali.toggle.worldchat")
        {
            id_pair.first = Input::Events::NAALI_TOGGLE_WORLDCHAT;
            id_pair.second = 0;
        }
        else if (name == "naali.unfocus.ui")
        {
            id_pair.first = Input::Events::UNFOCUS_UI;
            id_pair.second = 0;
        }
        else if (name == "python.run")
        {
            id_pair.first = Input::Events::PY_RUN_COMMAND;
            id_pair.second = 0;
        }
        else if (name == "python.restart")
        {
            id_pair.first = Input::Events::PY_RESTART;
            id_pair.second = 0;
        }
        else if (name == "python.duplicate.drag")
        {
            id_pair.first = Input::Events::PY_DUPLICATE_DRAG;
            id_pair.second = 0;
        }
        else if (name == "python.object.toggle.move")
        {
            id_pair.first = Input::Events::PY_OBJECTEDIT_TOGGLE_MOVE;
            id_pair.second = 0;
        }
        else if (name == "python.object.toggle.scale")
        {
            id_pair.first = Input::Events::PY_OBJECTEDIT_TOGGLE_SCALE;
            id_pair.second = 0;
        }
        else
        {
            id_pair.first = 0;
            id_pair.second = 0;
        }
*/
        return id_pair;
    }
}