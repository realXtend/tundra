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
    class ServerInfoProvider;
    class Session;
    class Settings;

    class Provider : public Communications::InWorldVoice::ProviderInterface
    {
        Q_OBJECT
    public:
        Provider(Foundation::Framework* framework, Settings* settings);
        virtual ~Provider();
        virtual Communications::InWorldVoice::SessionInterface* Session();
        virtual QString& Description();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);
        virtual QList<QString> Statistics();
    private:
        void CloseSession();
        Foundation::Framework* framework_;
        QString description_;
        MumbleVoip::Session* session_;  //! \todo Use shared ptr ...
        ServerInfoProvider* server_info_provider_;
        ServerInfo* server_info_;
        event_category_id_t networkstate_event_category_;
        Settings* settings_;

    private slots:
        void OnMumbleServerInfoReceived(ServerInfo info);
    };

} // MumbleVoip

#endif // incl_MumbleVoipModule_Provider_h
