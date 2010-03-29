// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ServiceGetter.h"
#include "InputServiceInterface.h"

namespace CoreUi
{
    ServiceGetter::ServiceGetter(Foundation::Framework *framework, QObject *parent) :
        QObject(parent),
        framework_(framework)
    {

    }

    void ServiceGetter::GetKeyBindings()
    {
        boost::shared_ptr<Foundation::InputServiceInterface> input_logic = framework_->GetServiceManager()->
            GetService<Foundation::InputServiceInterface>(Foundation::Service::ST_Input).lock();

        if (!input_logic.get())
            return;

        Foundation::KeyBindings *bindings = input_logic->GetBindings();
        if (bindings)
            emit KeyBindingsChanged(bindings);
    }

    void ServiceGetter::SetKeyBindings(Foundation::KeyBindings *bindings)
    {
        boost::shared_ptr<Foundation::InputServiceInterface> input_logic = framework_->GetServiceManager()->
            GetService<Foundation::InputServiceInterface>(Foundation::Service::ST_Input).lock();

        if (!input_logic.get())
            return;

        input_logic->SetBindings(bindings);
    }

    void ServiceGetter::RestoreKeyBindings()
    {
        boost::shared_ptr<Foundation::InputServiceInterface> input_logic = framework_->GetServiceManager()->
            GetService<Foundation::InputServiceInterface>(Foundation::Service::ST_Input).lock();

        if (!input_logic.get())
            return;

        input_logic->RestoreDefaultBindings();
    }

    void ServiceGetter::PublishChangedBindings(Foundation::KeyBindings *bindings)
    {
        emit KeyBindingsChanged(bindings);
    }
}
