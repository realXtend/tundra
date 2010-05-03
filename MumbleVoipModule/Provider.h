// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Provider_h
#define incl_MumbleVoipModule_Provider_h

#include <QObject>
#include "CommunicationsService.h"
#include "ServerInfo.h"

namespace Foundation
{
    class Framework;
    class EventDataInterface;
}

namespace MumbleVoip
{
    class ServerObserver;

    namespace InWorldVoice
    {
        class Session;

        class Provider : public Communications::InWorldVoice::ProviderInterface
        {
            Q_OBJECT
        public:
            Provider(Foundation::Framework* framework);
            virtual ~Provider();
            virtual Communications::InWorldVoice::SessionInterface* Session();
            virtual QString& Description();
            virtual void Update(f64 frametime);
            virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);
        private:
            Foundation::Framework* framework_;
            QString description_;
            MumbleVoip::InWorldVoice::Session* session_;
            ServerObserver* server_observer_;
            ServerInfo* server_info_;

        private slots:
            void OnMumbleServerInfoReceived(ServerInfo info);

        //signals:
        //    void SessionAvailable();
        };

    } // InWorldVoice

} // MumbleVoip

#endif // incl_MumbleVoipModule_Provider_h
