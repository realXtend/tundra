// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <OISKeyboard.h>

#include "ConsoleModule.h"
#include "ConsoleManager.h"
#include "InputEvents.h"
#include "InputServiceInterface.h"

namespace Console
{
    ConsoleModule::ConsoleModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    ConsoleModule::~ConsoleModule()
    {
    }

    // virtual
    void ConsoleModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void ConsoleModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void ConsoleModule::PreInitialize()
    {
        manager_ = ConsolePtr(new ConsoleManager(this));
    }

    // virtual
    void ConsoleModule::Initialize()
    {
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Console, manager_);
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_ConsoleCommand, checked_static_cast<ConsoleManager*>(manager_.get())->GetCommandManager());

        LogInfo("Module " + Name() + " initialized.");
    }

    void ConsoleModule::PostInitialize()
    {
        checked_static_cast<ConsoleManager*>(manager_.get())->CreateDelayed();
    }

    void ConsoleModule::Update(Core::f64 frametime)
    {
        {
            PROFILE(ConsoleModule_Update);

            assert (manager_);
            manager_->Update(frametime);
        }
        RESETPROFILER;
    }

    // virtual
    bool ConsoleModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (framework_->GetEventManager()->QueryEventCategory("Input") == category_id)
        {
            if (event_id == Input::Events::SCROLL)
            {
                if (manager_->IsVisible())
                {
                    int rel = checked_static_cast<Input::Events::SingleAxisMovement*>(data)->z_.rel_;
                    manager_->Scroll(rel);
                    return true;
                }
            }

            if (event_id == Input::Events::SHOW_DEBUG_CONSOLE)
            {
                manager_->SetVisible(!manager_->IsActive());
  
                return true;
            }

            if (event_id == Input::Events::SHOW_DEBUG_CONSOLE_REL)
            {
                boost::shared_ptr<Input::InputServiceInterface> input = framework_->GetService<Input::InputServiceInterface>(Foundation::Service::ST_Input).lock();
                if (input)
                {
                    if (manager_->IsActive())
                        input->SetState(Input::State_Buffered);
                    else
                        input->SetState();
                }

                return true;
            }

            if (event_id == Input::Events::BUFFERED_KEY_PRESSED || event_id == Input::Events::BUFFERED_KEY_RELEASED)
            {
                if (manager_->IsActive())
                {
                    int code = checked_static_cast<Input::Events::BufferedKey*>(data)->code_;
                    unsigned int text = checked_static_cast<Input::Events::BufferedKey*>(data)->text_;

                    if (event_id == Input::Events::BUFFERED_KEY_PRESSED)
                        return manager_->HandleKeyDown(code, text);
                    else
                        return manager_->HandleKeyUp(code, text);
                }
            }
        }

        return false;
    }

    // virtual 
    void ConsoleModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(manager_);
        framework_->GetServiceManager()->UnregisterService(checked_static_cast< ConsoleManager* >(manager_.get())->GetCommandManager());

        assert (manager_);
        manager_.reset();

        LogInfo("Module " + Name() + " uninitialized.");
    }
}


