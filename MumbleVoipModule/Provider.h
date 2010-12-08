// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Provider_h
#define incl_MumbleVoipModule_Provider_h

#include <QObject>
#include <QMap>
#include <QString>
#include "CommunicationsService.h"
#include "ServerInfo.h"
#include "AttributeChangeType.h"

class UiProxyWidget;
class IEventData;
class QSignalMapper;
class EC_VoiceChannel;

namespace Foundation
{
    class Framework;
}
namespace Scene
{
    class Entity;
}
class IComponent;

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

    private slots:
        void OnECAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change);
        void OnECVoiceChannelDestroyed(QObject* obj);
        void OnSceneAdded(const QString &name);

    private:
        void CreateSession();
        void CloseSession();
        QString GetUsername();

        Foundation::Framework* framework_;
        QString description_;
        MumbleVoip::Session* session_;  //! \todo Use shared ptr ...
        ServerInfoProvider* server_info_provider_;
        event_category_id_t networkstate_event_category_;
        event_category_id_t framework_event_category_;
        Settings* settings_;
        QWidget* microphone_adjustment_widget_;
        QList<EC_VoiceChannel*> ec_voice_channels_;
        QMap<EC_VoiceChannel*, QString> channel_names_;
        QSignalMapper* signal_mapper_;

    private slots:
        void OnMumbleServerInfoReceived(ServerInfo info);
        void OnMicrophoneAdjustmentWidgetDestroyed();
        void ECVoiceChannelChanged(const QString &channelname);
    };

} // MumbleVoip

#endif // incl_MumbleVoipModule_Provider_h
