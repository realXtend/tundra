// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <OIS.h>

#include "InputModuleOIS.h"

namespace Input
{
    InputModuleOIS::InputModuleOIS() : ModuleInterfaceImpl(type_static_)
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

        OIS::ParamList pl;

        //size_t windowHnd = 0;
        //std::ostringstream windowHndStr;
        //win->getCustomAttribute("WINDOW", &windowHnd);
        //windowHndStr << windowHnd;
        //pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

#if defined OIS_WIN32_PLATFORM
        pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_BACKGROUND" )));
        pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
        pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
        pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
#endif

//        OIS::InputManager *inputManager = OIS::InputManager::createInputSystem( pl );

        OIS::InputManager *inputManager = OIS::InputManager::createInputSystem( 0 );

        OIS::Keyboard *Keyboard = static_cast<OIS::Keyboard*>(inputManager->createInputObject( OIS::OISKeyboard, false ));
        OIS::Mouse *mouse = static_cast<OIS::Mouse*>(inputManager->createInputObject( OIS::OISMouse, false ));
        //try
        //{
        //    Joy = static_cast<OIS::JoyStick*>(InputManager->createInputObject( OIS::OISJoyStick, bufferedJoy ));
        //}
        //catch(...)
        //{
        //    Joy = 0;
        //}
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void InputModuleOIS::Uninitialize()
    {
        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual 
    void InputModuleOIS::Update(Core::f64 frametime)
    {
    }
}

