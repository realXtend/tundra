// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_SessionManager_h
#define incl_Communication_SessionManager_h

#include "MasterWidget.h"
#include "UiDefines.h"

#include <QMenuBar>
#include <QMenu>
#include <QObject>

#include "ui_SessionManagerWidget.h"
#include "interface.h"

namespace UiHelpers
{
    class SessionManager : public QObject
    {

    Q_OBJECT
    Q_PROPERTY(UiDefines::PresenceStatus::PresenceState presence_status_ READ GetPresenceStatus)
        
    public:
        SessionManager(CommunicationUI::MasterWidget *parent);
        virtual ~SessionManager();

        void SetupUi(Ui::SessionManagerWidget *session_manager_ui) { SAFE_DELETE(session_manager_ui_); session_manager_ui_ = session_manager_ui; }
        void Start(const QString &username, Communication::ConnectionInterface *im_connection);

        UiDefines::PresenceStatus::PresenceState GetPresenceStatus() { return presence_status_; }

        
    private:
        QMenuBar *ConstructMenuBar();
        QMenuBar *menu_bar_;

        Ui::SessionManagerWidget *session_manager_ui_;
        Communication::ConnectionInterface *im_connection_;
        CommunicationUI::MasterWidget *parent_;

        UiDefines::PresenceStatus::PresenceState presence_status_;

    private slots:
        void Hide() { parent_->hide(); }
        void Logout() { emit StateChange(UiDefines::UiStates::Disconnected); }
        void Exit() { emit StateChange(UiDefines::UiStates::Exit); }
        
        void StatusHandler(UiDefines::PresenceStatus::PresenceState new_status);
        void StatusAvailable() {}
        void StatusAway() {}
        void StatusBusy() {}
        void StatusOffline() {}

        void VoiceSessionRecieved(Communication::VoiceSessionInterface& voice_session);

    signals:
        void StateChange(UiDefines::UiStates::ConnectionState);
        void StatusChange(UiDefines::PresenceStatus::PresenceState);

    };
}

#endif // incl_Communication_SessionManager_h