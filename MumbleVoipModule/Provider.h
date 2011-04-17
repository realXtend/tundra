// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Provider_h
#define incl_MumbleVoipModule_Provider_h

#include <QObject>
#include <QMap>
#include "CommunicationsService.h"
#include "ServerInfo.h"
#include "AttributeChangeType.h"

namespace TundraLogic
{
    class TundraLogicModule;
}

class UiProxyWidget;
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
    class Provider : public Communications::InWorldVoice::ProviderInterface
    {
        Q_OBJECT
    public:
        Provider(Foundation::Framework* framework, Settings* settings);
        virtual ~Provider();
        void PostInitialize();

    public slots:
        bool HasSession();
        virtual Communications::InWorldVoice::SessionInterface* Session();

        virtual QString& Description();
        virtual void Update(f64 frametime);
        virtual QList<QString> Statistics();
        virtual void ShowMicrophoneAdjustmentDialog();

    private slots:
        void OnECAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change);
        void OnECVoiceChannelDestroyed(QObject* obj);
        void OnSceneAdded(const QString &name);

    private:
        void CreateSession();
        void CloseSession();
        void CheckChannelQueue();
        QString GetUsername();
        QString GetAvatarUuid();
        void AddECVoiceChannel(EC_VoiceChannel* channel);

        Foundation::Framework* framework_;
        QString description_;
        MumbleVoip::Session* session_;  /// \todo Use shared ptr ...
        Settings* settings_;
        QWidget* microphone_adjustment_widget_;
        QList<EC_VoiceChannel*> ec_voice_channels_;
        QMap<EC_VoiceChannel*, QString> channel_names_;
        QSignalMapper* signal_mapper_;

        boost::shared_ptr<TundraLogic::TundraLogicModule> tundra_logic_;
        QList<EC_VoiceChannel*> channel_queue_;

    private slots:
        void OnMicrophoneAdjustmentWidgetDestroyed();
        void ECVoiceChannelChanged(const QString &channelname);
    };

} // MumbleVoip

#endif // incl_MumbleVoipModule_Provider_h
