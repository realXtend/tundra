// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Service.h"

#include "CommunicationModule.h"
#include "NetworkEvents.h"
#include "EventManager.h"
#include "Framework.h"

#include "MemoryLeakCheck.h"

namespace Communications
{
    boost::shared_ptr<Service> Service::instance_;

    Service::Service() :
        in_world_voice_provider_(0),
        in_world_chat_provider_(0)
    {
    }

    Service::~Service()
    {
    }

    boost::shared_ptr<Service> Service::IntancePtr()
    {
        if (Service::instance_.get() == 0)
        {
            Service::instance_ = boost::shared_ptr<Service>(new Service());
        }
        return Service::instance_;
    }

    //ServiceInterface* Service::Instance()
    //{
    //    if (!Service::instance_)
    //    {
    //        Service::instance_ = new Service();
    //    }
    //    return Service::instance_;
    //}

    //ServiceWeakPtr ServiceInterface::IntanceWeakPtr()
    //{

    //}


    InWorldVoice::SessionInterface* Service::InWorldVoiceSession() const
    {
        if (!in_world_voice_provider_)
            return 0;
        else
            return in_world_voice_provider_->Session();
    }

    InWorldChat::SessionInterface* Service::InWorldChatSession() const
    {
        if (!in_world_chat_provider_)
            return 0;
        else
            return in_world_chat_provider_->Session();
    }

    void Service::Register(InWorldVoice::ProviderInterface& provider)
    {
        in_world_voice_provider_ = &provider;
//        connect(in_world_voice_provider_, SIGNAL(SessionUnavailable()), SLOT(
  //      InWorldVoiceUnvailable
        QString message = QString("In-world voice provider [%1] registered.").arg(provider.Description());
        Communication::CommunicationModule::LogInfo(message.toStdString());
        emit Communications::ServiceInterface::InWorldVoiceAvailable();
    }

    void Service::Register(InWorldChat::ProviderInterface& provider)
    {
        in_world_chat_provider_ = &provider;
        emit InWorldChatAvailable();
    }

} // Communications
