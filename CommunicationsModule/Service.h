// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CommunicationsModuole_Service_h
#define incl_CommunicationsModuole_Service_h

#include "CommunicationsService.h"

namespace CommunicationsService
{
    class Service : public Communications::ServiceInterface
    {
        Q_OBJECT
    private:
        // Private constructor
        Service(); 

    public:
        // Default deconstructor
        virtual ~Service();

    public slots:
//        static ServiceInterface* Instance();
        static boost::shared_ptr<Service> IntancePtr();

    
        virtual Communications::InWorldVoice::SessionInterface* InWorldVoiceSession() const;
        virtual Communications::InWorldChat::SessionInterface* InWorldChatSession() const;

        //! Registration methods for communication providers
        virtual void Register(Communications::InWorldVoice::ProviderInterface& provider);
        virtual void Register(Communications::InWorldChat::ProviderInterface& provider);

    private:
        static boost::shared_ptr<Service> instance_;
        Communications::InWorldVoice::ProviderInterface* in_world_voice_provider_;
        Communications::InWorldChat::ProviderInterface* in_world_chat_provider_;
    };

} // CommunicationsService

#endif // incl_CommunicationsModuole_Service_h
