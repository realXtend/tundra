// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Provider_h
#define incl_MumbleVoipModule_Provider_h

#include <QObject>
#include "CommunicationsService.h"
#include "ServerInfo.h"

namespace Foundation
{
    class Framework;
}

namespace MumbleVoip
{
    class ConnectionManager;
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
        private:
            QString description_;
            InWorldVoice::Session* session_;
            ConnectionManager* connection_manager_;
            ServerObserver* server_observer_;
            ServerInfo* server_info_;

        private slots:
            void OnMumbleServerInfoReceived(ServerInfo info);

        };

    } // InWorldVoice

} // MumbleVoip

#endif // incl_MumbleVoipModule_Provider_h
