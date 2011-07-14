// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Provider_h
#define incl_MumbleVoipModule_Provider_h

#include <QObject>
#include "CommunicationsService.h"
#include "ServerInfo.h"

namespace TundraLogic
{
    class TundraLogicModule;
}

class UiProxyWidget;
class IEventData;
class QSignalMapper;

namespace Foundation
{
    class Framework;
}

namespace MumbleVoip
{
    class Session;
    class Settings;

    typedef boost::shared_ptr<Session> SessionPtr;

    /// Provides Mumble implementation of InWorldVoiceSession objects
    ///
    class Provider : public Communications::InWorldVoice::ProviderInterface
    {
        Q_OBJECT
    public:
        Provider(Foundation::Framework* framework, Settings* settings);
        virtual ~Provider();
        void PostInitialize();
        SessionPtr GetSession() const { return session_; }

    public slots:
        virtual Communications::InWorldVoice::SessionInterface* Session();
        virtual QString& Description();
        virtual void Update(f64 frametime);
        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);
        virtual QList<QString> Statistics();
        virtual void ShowMicrophoneAdjustmentDialog();

    private:
        void CreateSession();
        void CloseSession();

        SessionPtr session_;
        Settings* settings_;

        Foundation::Framework* framework_;
        QString description_;
        event_category_id_t networkstate_event_category_;

        QWidget* microphone_adjustment_widget_;
        boost::shared_ptr<TundraLogic::TundraLogicModule> tundra_logic_;

    private slots:
        void OnMicrophoneAdjustmentWidgetDestroyed();
    };

} // MumbleVoip

#endif // incl_MumbleVoipModule_Provider_h
