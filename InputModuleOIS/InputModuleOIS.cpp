// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "RendererEvents.h"
#include "InputModuleOIS.h"
#include "BufferedKeyboard.h"
#include "Mapper.h"

namespace Input
{
    const char *DeviceType[6] = {"OISUnknown", "OISKeyboard", "OISMouse", "OISJoyStick",
							 "OISTablet", "OISOther"};

    InputModuleOIS::InputModuleOIS() : 
        ModuleInterfaceImpl(type_static_)
        , input_manager_(0)
        , keyboard_(0)
        , mouse_(0)
        , joy_(0)
        , event_category_(0)
        , dragged_(false)
        , input_state_(State_Unknown)
    {       
    }

    InputModuleOIS::~InputModuleOIS()
    {
    }

    // virtual
    void InputModuleOIS::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void InputModuleOIS::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void InputModuleOIS::Initialize()
    {
        LogInfo("*** Initializing OIS ***");

        if (framework_->GetServiceManager()->IsRegistered(Foundation::Service::ST_Renderer) == false)
        {
            LogError("Failed to initialize. No renderer service registered.");
            return;
        }
        Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer);
        size_t window_handle = renderer->GetWindowHandle();
        if (window_handle == 0)
        {
            LogError("Failed to initialize. No open window.");
            return;
        }
        
        event_category_ = framework_->GetEventManager()->RegisterEventCategory("Input");

        OIS::ParamList pl;
        pl.insert(std::make_pair(std::string("WINDOW"), Core::ToString(window_handle)));

#if defined OIS_WIN32_PLATFORM
//        pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_BACKGROUND" )));
        pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
        pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
        pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
        pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
#endif

        // throws exception if fails, not handled since it should be rather fatal
        input_manager_ = OIS::InputManager::createInputSystem( pl );

        buffered_keyboard_ = BufferedKeyboardPtr(new BufferedKeyboard(this));
        keyboard_ = static_cast<OIS::Keyboard*>(input_manager_->createInputObject( OIS::OISKeyboard, false ));      
        LogInfo("Keyboard input initialized.");
   
        mouse_ = static_cast<OIS::Mouse*>(input_manager_->createInputObject( OIS::OISMouse, false ));
        LogInfo("Mouse input initialized.");
        try
        {
            joy_ = static_cast<OIS::JoyStick*>(input_manager_->createInputObject( OIS::OISJoyStick, false ));
            LogInfo("Joystick / gamepad input initialized.");
        }
        catch(...)
        {
            LogInfo("Joystick / gamepad not found.");
        }

        // set window default width / height. Should be updated when window size changes
        WindowResized(renderer->GetWindowWidth(), renderer->GetWindowHeight());

        unsigned int v = input_manager_->getVersionNumber();

        std::stringstream ss;
	    ss << "OIS Version: " << (v>>16 ) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF);
        LogInfo(ss.str());

#if defined OIS_WIN32_PLATFORM
        LogInfo("Release Name: " + input_manager_->getVersionName());
        LogInfo("Manager: " + input_manager_->inputSystemName());
        LogInfo("Total Keyboards: " + Core::ToString(input_manager_->getNumberOfDevices(OIS::OISKeyboard)));
        LogInfo("Total Mice: " + Core::ToString(input_manager_->getNumberOfDevices(OIS::OISMouse)));
        LogInfo("Total JoySticks: " + Core::ToString(input_manager_->getNumberOfDevices(OIS::OISJoyStick)));
        
        //List all devices
        OIS::DeviceList list = input_manager_->listFreeDevices();
        for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
        LogInfo("\tDevice: " + std::string(DeviceType[i->first]) + " Vendor: " + i->second);
#endif

        key_mapping_ = MapperPtr(new Mapper(this));

        GetFramework()->GetServiceManager()->RegisterService(Foundation::Service::ST_Input, key_mapping_.get());


        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void InputModuleOIS::Uninitialize()
    {
        WindowClosed();

        GetFramework()->GetServiceManager()->UnregisterService(key_mapping_.get());
        key_mapping_.reset();

        listened_keys_.clear();
        sliders_.clear();


        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual 
    void InputModuleOIS::Update(Core::f64 frametime)
    {
        // should be first, so buffered keyboard events get launched first
        if (buffered_keyboard_)
            buffered_keyboard_->Update();

        if( keyboard_ && mouse_ )
        {
            keyboard_->capture();
	        mouse_->capture();
    	    if ( joy_ ) joy_->capture();

            const OIS::MouseState &ms = mouse_->getMouseState();
            if (ms.Z.rel != 0)
            {
                Events::SingleAxisMovement mw;
                mw.z_.rel_ = ms.Z.rel;
                mw.z_.abs_ = ms.Z.abs;
                framework_->GetEventManager()->SendEvent(event_category_, Events::SCROLL, &mw);
            }
            // previous mouse position
            const Events::Movement prev_movement = movement_;

            // we assume GetMouseMovement() will be called several times in one frame
            // It makes sense then to only query OIS once for the mouse state and then
            // return the same state per frame.
            movement_.x_.rel_ = ms.X.rel;
            movement_.x_.abs_ = ms.X.abs;

            movement_.y_.rel_ = ms.Y.rel;
            movement_.y_.abs_ = ms.Y.abs;

            movement_.z_.rel_ = ms.Z.rel;
            movement_.z_.abs_ = ms.Z.abs;

            // launch event when mouse is dragged. Disabled since launching event every frame is not all that efficient.
            //if (ms.buttonDown(OIS::MB_Right))
            //{
            //    //if (movement_ != prev_movement)
            //    {
            //        dragged_ = true;
            //        framework_->GetEventManager()->SendEvent(event_category_, Events::DRAGGING, &movement_);
            //    }
            //} else if (dragged_)
            //{
            //    dragged_ = false;
            //    Events::Movement no_rel_movement = movement_;
            //    no_rel_movement.x_.rel_ = 0;
            //    no_rel_movement.y_.rel_ = 0;
            //    no_rel_movement.z_.rel_ = 0;
            //    framework_->GetEventManager()->SendEvent(event_category_, Events::DRAGGING_STOPPED, &no_rel_movement);
            //}

            // first update input events for the current state, then send events for keys that are common to all states
            UpdateSliderEvents(input_state_);
            UpdateSliderEvents(Input::State_All);
            SendKeyEvents(input_state_);
            SendKeyEvents(Input::State_All);
        }
    }

    // virtual
    bool InputModuleOIS::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        bool handled = false;

        if (framework_->GetEventManager()->QueryEventCategory("Renderer") == category_id)
        {
            if (event_id == OgreRenderer::Event::WINDOW_CLOSED)
                WindowClosed();

            if (event_id == OgreRenderer::Event::WINDOW_RESIZED)
            {
                WindowResized(  checked_static_cast<OgreRenderer::Event::WindowResized*>(data)->width_,
                                checked_static_cast<OgreRenderer::Event::WindowResized*>(data)->height_ );
            }
        }
        if (event_category_ == category_id)
        {
            if (event_id == Input::Events::INPUTSTATE_FIRSTPERSON)
            {
                SetState(Input::State_FirstPerson);
            } else if (event_id == Input::Events::INPUTSTATE_THIRDPERSON)
            {
                SetState(Input::State_ThirdPerson);
            } else if (event_id == Input::Events::INPUTSTATE_FREECAMERA)
            {
                SetState(Input::State_FreeCamera);
            }
        }

        // no need to mark events handled
        return handled;
    }

    bool InputModuleOIS::IsKeyDown(OIS::KeyCode keycode) const
    {
        return keyboard_ ? keyboard_->isKeyDown(keycode) : false;
    }

    bool InputModuleOIS::IsButtonDown(OIS::MouseButtonID code) const
    {
        return mouse_->getMouseState().buttonDown(code);
    }

    boost::optional<const Events::Movement&> InputModuleOIS::GetDraggedSliderInfo(Core::event_id_t dragged_event)
    {
        SliderInfoVector &sliders = GetSliderInfo(input_state_);
        for (size_t i=0 ; i<sliders.size() ; ++i)
        {
            if (sliders[i].dragged_event_ == dragged_event && sliders[i].dragged_)
            {
                return boost::optional<const Events::Movement&>(sliders[i].movement_);
            }
        }
        return boost::optional<const Events::Movement&>();
    }

    void InputModuleOIS::UpdateSliderEvents(Input::State state)
    {
        const OIS::MouseState &ms = mouse_->getMouseState();
        SliderInfoVector &sliders = GetSliderInfo(state);

        for (size_t i=0 ; i<sliders.size() ; ++i)
        {
            sliders[i].dragged_ = false;
            if (sliders[i].slider_ == SliderMouse)
            {
                if (sliders[i].button_ == -1 || ms.buttonDown(static_cast<OIS::MouseButtonID>(sliders[i].button_)))
                {
                    // check modifiers in a bit convoluted way. All combos of ctrl+a, ctrl+alt+a and ctrl+alt+shift+a must work!
                    if (((sliders[i].modifier_ & OIS::Keyboard::Alt)   == 0 || keyboard_->isModifierDown(OIS::Keyboard::Alt))  &&
                        ((sliders[i].modifier_ & OIS::Keyboard::Ctrl)  == 0 || keyboard_->isModifierDown(OIS::Keyboard::Ctrl)) &&
                        ((sliders[i].modifier_ & OIS::Keyboard::Shift) == 0 || keyboard_->isModifierDown(OIS::Keyboard::Shift))) 
                    {
                        sliders[i].dragged_ = true;
                        sliders[i].movement_ = movement_;
                    }
                }
            }
        }
    }

    void InputModuleOIS::SendKeyEvents(Input::State state)
    {
        KeyEventInfoVector &keys = GetKeyInfo(state);

        const bool alt = keyboard_->isModifierDown(OIS::Keyboard::Alt);
        const bool ctrl = keyboard_->isModifierDown(OIS::Keyboard::Ctrl);
        const bool shift = keyboard_->isModifierDown(OIS::Keyboard::Shift);
        for (size_t i=0 ; i<keys.size() ; ++i)
        {
            bool key_released = false;
            if (keyboard_->isKeyDown(keys[i].key_))
            {
                const bool mod_alt   = !((keys[i].modifier_ & OIS::Keyboard::Alt)   == 0);
                const bool mod_ctrl  = !((keys[i].modifier_ & OIS::Keyboard::Ctrl)  == 0);
                const bool mod_shift = !((keys[i].modifier_ & OIS::Keyboard::Shift) == 0);

                if(!keys[i].pressed_ && buffered_keyboard_->IsKeyHandled(keys[i].key_) == false)
                {
                    // check modifiers in a bit convoluted way. All combos of ctrl+a, ctrl+alt+a and ctrl+alt+shift+a must work!
                    if ( ((mod_alt   && alt)   || (!mod_alt   && !alt))  &&
                         ((mod_ctrl  && ctrl)  || (!mod_ctrl  && !ctrl)) &&
                         ((mod_shift && shift) || (!mod_shift && !shift)) )
                    {
                        keys[i].pressed_ = true;
                        framework_->GetEventManager()->SendEvent(event_category_, keys[i].pressed_event_id_, NULL);
                    } else
                    {
                        key_released = true;
                    }
                } else
                {
                    // see if any modifiers were released or pressed
                    if ( ((mod_alt   && !alt)   || (!mod_alt   && alt))  ||
                         ((mod_ctrl  && !ctrl)  || (!mod_ctrl  && ctrl)) ||
                         ((mod_shift && !shift) || (!mod_shift && shift)) )
                    {
                        key_released = true;
                    }
                }
            }
            else if(keys[i].pressed_)
            {
                // key no longer held down
                key_released = true;
            }

            if (key_released)
            {
                keys[i].pressed_ = false;
                framework_->GetEventManager()->SendEvent(event_category_, keys[i].released_event_id_, NULL);
            }
        }
    }

    void InputModuleOIS::WindowClosed()
    {
        if( input_manager_ )
        {
            input_manager_->destroyInputObject( mouse_ );
            input_manager_->destroyInputObject( keyboard_ );
            input_manager_->destroyInputObject( joy_ );

            OIS::InputManager::destroyInputSystem(input_manager_);

            input_manager_ = 0;
            mouse_ = 0;
            keyboard_ = 0;
            joy_ = 0;

            buffered_keyboard_.reset();
        }
    }

    void InputModuleOIS::WindowResized(int width, int height)
    {
        if (mouse_)
        {
            const OIS::MouseState &ms = mouse_->getMouseState();
            ms.width = width;
            ms.height = height;
        }
    }

    
    void InputModuleOIS::RegisterUnbufferedKeyEvent(Input::State state, OIS::KeyCode key, Core::event_id_t pressed_event, Core::event_id_t released_event, int modifier)
    {
        assert (pressed_event + 1 == released_event);

        UnBufferedKeyEventInfo keyeventinfo;
        keyeventinfo.pressed_ = false;
        keyeventinfo.pressed_event_id_ = pressed_event;
        keyeventinfo.released_event_id_ = released_event;
        keyeventinfo.key_ = key;
        keyeventinfo.modifier_ = modifier;

#ifdef _DEBUG
        const bool mod_alt   = !((modifier & OIS::Keyboard::Alt)   == 0);
        const bool mod_ctrl  = !((modifier & OIS::Keyboard::Ctrl)  == 0);
        const bool mod_shift = !((modifier & OIS::Keyboard::Shift) == 0);
        std::string mod_str;
        if (!mod_alt && !mod_ctrl && !mod_shift)
            mod_str = " none";
        if (mod_alt)
            mod_str += " alt";
        if (mod_ctrl)
            mod_str += " ctrl";
        if (mod_shift)
            mod_str += " shift";

        LogDebug("Bound key " + keyboard_->getAsString(key) + " to event id: " + Core::ToString(pressed_event) + " with modifiers:" + mod_str + ".");
#endif

        KeyEventInfoMap::iterator key_vector = listened_keys_.find(state);
        if (key_vector == listened_keys_.end())
        {
            listened_keys_[state].push_back(keyeventinfo);
        } else
        {
            KeyEventInfoVector::iterator it = std::find(key_vector->second.begin(), key_vector->second.end(), keyeventinfo);
            if ( it != key_vector->second.end())
            {
                // replace old event
                *it = keyeventinfo;

                LogDebug("Replaced previously bound key.");
            } else
            {
                // register new event
                key_vector->second.push_back(keyeventinfo);
            }
        }
    }

    void InputModuleOIS::RegisterSliderEvent(Input::State state, Slider slider, Core::event_id_t dragged_event, Core::event_id_t stopped_event, int button, int modifier)
    {
        assert (dragged_event + 1 == stopped_event);

        SliderEventInfo info;
        info.dragged_ = false;
        info.dragged_event_ = dragged_event;
        info.stopped_event_ = stopped_event;
        info.slider_ = slider;
        info.button_ = button;
        info.modifier_ = modifier;

        SliderInfoMap::iterator slider_vector = sliders_.find(state);
        if (slider_vector == sliders_.end())
        {
            sliders_[state].push_back(info);
        } else
        {
            SliderInfoVector::iterator it = std::find(slider_vector->second.begin(), slider_vector->second.end(), info);
            if ( it != slider_vector->second.end())
            {
                // replace old event
                *it = info;
            } else
            {
                // register new event
                slider_vector->second.push_back(info);
            }
        }
    }

    InputModuleOIS::KeyEventInfoVector &InputModuleOIS::GetKeyInfo(Input::State state)
    {
        KeyEventInfoMap::iterator it = listened_keys_.find(state);
        if (it == listened_keys_.end())
        {
            listened_keys_[state] = KeyEventInfoVector();
            return GetKeyInfo(state);
        }
        return it->second;
    }

    InputModuleOIS::SliderInfoVector &InputModuleOIS::GetSliderInfo(Input::State state)
    {
        SliderInfoMap::iterator it = sliders_.find(state);
        if (it == sliders_.end())
        {
            sliders_[state] = SliderInfoVector();
            return GetSliderInfo(state);
        }
        return it->second;
    }
}

