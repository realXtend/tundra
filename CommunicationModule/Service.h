// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_Service_h
#define incl_Interfaces_Service_h

#include "CommunicationsService.h"

namespace Communications
{
    class Service : public ServiceInterface
    {
        Q_OBJECT
    private:
        // Private constructor
        Service(); 

    public:
        // Default deconstructor
        virtual ~Service();

//        static ServiceInterface* Instance();
        static boost::shared_ptr<Service> IntancePtr();

        virtual InWorldVoice::SessionInterface* InWorldVoiceSession() const;
        virtual InWorldChat::SessionInterface* InWorldChatSession() const;

        //! Registration methods for communication providers
        virtual void Register(InWorldVoice::ProviderInterface& provider);
        virtual void Register(InWorldChat::ProviderInterface& provider);

    private:
//        static Service* instance_;
        static boost::shared_ptr<Service> instance_;
        InWorldVoice::ProviderInterface* in_world_voice_provider_;
        InWorldChat::ProviderInterface* in_world_chat_provider_;
    };
}

#endif incl_Interfaces_Service_h
