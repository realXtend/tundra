// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "RendererEvents.h"
#include "InputModuleOIS.h"
#include "InputEvents.h"
#include "BufferedKeyboard.h"

namespace Input
{
    InputModuleOIS::InputModuleOIS() : 
        ModuleInterfaceImpl(type_static_)
        , input_manager_(0)
        , keyboard_(0)
        , mouse_(0)
        , joy_(0)
        , event_category_(0)
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
        size_t window_handle = framework_->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer)->GetWindowHandle();
        if (window_handle == 0)
        {
            LogError("Failed to initialize. No open window.");
            return;
        }
        
        event_category_ = framework_->GetEventManager()->RegisterEventCategory("Input");

        OIS::ParamList pl;
        pl.insert(std::make_pair(std::string("WINDOW"), Core::ToString(window_handle)));

#if defined OIS_WIN32_PLATFORM
        pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_BACKGROUND" )));
        pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
        pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
        pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
#endif

        input_manager_ = OIS::InputManager::createInputSystem( pl );

        buffered_keyboard_ = BufferedKeyboardPtr(new BufferedKeyboard(framework_));

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

        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void InputModuleOIS::Uninitialize()
    {
        WindowClosed();

        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual 
    void InputModuleOIS::Update(Core::f64 frametime)
    {
        if( keyboard_ && mouse_ )
        {
            keyboard_->capture();
	        mouse_->capture();
    	    if ( joy_ ) joy_->capture();

            const OIS::MouseState &ms = mouse_->getMouseState();
            if (ms.Z.rel != 0)
            {
                Events::MouseWheel mw(ms.Z.rel, ms.Z.abs);
                framework_->GetEventManager()->SendEvent(event_category_, Events::SCROLL, &mw);
            }
        }
        if (buffered_keyboard_)
            buffered_keyboard_->Update();
    }

    // virtual
    bool InputModuleOIS::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (framework_->GetEventManager()->QueryEventCategory("Renderer") == category_id)
        {
            if (event_id == OgreRenderer::Event::WINDOW_CLOSED)
                WindowClosed();
        }

        // no need to mark events handled
        return false;
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
}

