// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Provider_h
#define incl_MumbleVoipModule_Provider_h

#include <QObject>
#include "CommunicationsService.h"
#include "ServerInfo.h"

class UiProxyWidget;
class IEventData;

namespace Foundation
{
    class Framework;
}

namespace MumbleVoip
{
    class ServerInfoProvider;
    class Session;
    class Settings;

    /// Provides Mumble implementation of InWorldVoiceSession objects
    ///
    class Provider : public Communications::InWorldVoice::ProviderInterface
    {
        Q_OBJECT
    public:
        Provider(Foundation::Framework* framework, Settings* settings);
        virtual ~Provider();
    public slots:
        virtual Communications::InWorldVoice::SessionInterface* Session();
        virtual QString& Description();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);
        virtual QList<QString> Statistics();

        virtual void ShowMicrophoneAdjustmentDialog();
    private:
        void CloseSession();
        Foundation::Framework* framework_;
        QString description_;
        MumbleVoip::Session* session_;  //! \todo Use shared ptr ...
        ServerInfoProvider* server_info_provider_;
        ServerInfo* server_info_;
        event_category_id_t networkstate_event_category_;
        Settings* settings_;
        QWidget* microphone_adjustment_widget_;

    private slots:
        void OnMumbleServerInfoReceived(ServerInfo info);
        void OnMicrophoneAdjustmentWidgetDestroyed();
    };

} // MumbleVoip

#endif // incl_MumbleVoipModule_Provider_h
