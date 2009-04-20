// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "BufferedKeyboard.h"
#include "InputEvents.h"
#include "InputModuleOIS.h"

namespace Input
{
    BufferedKeyboard::BufferedKeyboard(InputModuleOIS *module) : 
        module_(module)
        , framework_(module->GetFramework())
        , input_manager_(0)
        , keyboard_(0)
        , event_category_(0)
    {

        size_t window_handle = framework_->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer)->GetWindowHandle();
        event_category_ = framework_->GetEventManager()->QueryEventCategory("Input");

        input_manager_ = OIS::InputManager::createInputSystem( window_handle );
        keyboard_ = static_cast<OIS::Keyboard*>(input_manager_->createInputObject( OIS::OISKeyboard, true ));
        keyboard_->setEventCallback(this);
    }

    BufferedKeyboard::~BufferedKeyboard()
    {
        if( input_manager_ )
        {
            input_manager_->destroyInputObject( keyboard_ );

            OIS::InputManager::destroyInputSystem(input_manager_);

            input_manager_ = 0;
            keyboard_ = 0;
        }
    }

    bool BufferedKeyboard::keyPressed( const OIS::KeyEvent &arg )
    {
        Events::BufferedKey key_event(arg.key, arg.text);
        bool handled = framework_->GetEventManager()->SendEvent(event_category_, Events::KEY_PRESSED, &key_event);

        if (handled)
            module_->SetHandledKey(arg.key);

        return handled;
    }
    bool BufferedKeyboard::keyReleased( const OIS::KeyEvent &arg )
    {
        Events::BufferedKey key_event(arg.key, arg.text);
        bool handled = framework_->GetEventManager()->SendEvent(event_category_, Events::KEY_RELEASED, &key_event);

        module_->SetHandledKey(OIS::KC_UNASSIGNED);

        return handled;
    }
}
