// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_SessionManager_h
#define incl_Communication_SessionManager_h

#include "UiDefines.h"

#include "EventHandler.h"

#include <QMenuBar>
#include <QMenu>
#include <QObject>

#include "ui_SessionManagerWidget.h"
#include "ui_SpatialVoiceConfigureWidget.h"

#include "TelepathyIMModuleFwd.h"

namespace Foundation
{
    class Framework;
}

namespace UiManagers
{
    class SessionManager : public QObject
    {

    Q_OBJECT
        
    public:
        SessionManager(CommunicationUI::MasterWidget *parent, Foundation::Framework *framework);
        virtual ~SessionManager();

        //! Setup ui
        void SetupUi(Ui::SessionManagerWidget *session_manager_ui) { SAFE_DELETE(session_manager_ui_); session_manager_ui_ = session_manager_ui; }
        //! Start the ui manager
        void Start(const QString &username, Communication::ConnectionInterface *im_connection, CommunicationUI::EventHandler *event_handler);      
        //! Hide Friend List Widget
        void HideFriendListWidget();

    private:
        QWidget *spatial_voice_manager_widget_;
        QMap<QString, QString> avatar_map_;
        QMenuBar *menu_bar_;
        QAction *available_status, *chatty_status, *away_status;
        QAction *extended_away_status, *busy_status, *hidden_status;
        QAction *set_status_message, *signout, *manage_spatial_voice, *join_chat_room, *add_new_friend;

        Ui::SessionManagerWidget            *session_manager_ui_;
        Ui::SpatialVoiceConfigureWidget     spatial_voice_configure_ui_;

        UiHelpers::SessionHelper            *session_helper_;
        Communication::ConnectionInterface  *im_connection_;
        CommunicationUI::MasterWidget       *main_parent_;
        CommunicationUI::FriendListWidget   *friend_list_widget_;
        CommunicationUI::EventHandler       *event_handler_;

        Foundation::Framework *framework_;

    public slots:
        void StatusChangedOutSideMenuBar(const QString &status_code);
        void UpdateAvatarList();
        void JoinChatRoomInputGiven();
        void StartTrackingSelectedAvatar();
        void StopTrackingSelectedAvatar();

    private slots:
        QMenuBar *ConstructMenuBar();
        void CreateFriendListWidget();

        void SignOut();
        void Show3DSoundManager();
        void Hide();
        void StatusAvailable();
        void StatusChatty();
        void StatusAway();
        void StatusExtendedAway();
        void StatusBusy();
        void StatusHidden();

        void ToggleShowFriendList();
        void JoinChatRoom();

        void ChatSessionReceived(Communication::ChatSessionInterface& chat_session);
        void VideoSessionReceived(Communication::VoiceSessionInterface& voice_session);

    signals:
        void StateChange(UiDefines::UiStates::ConnectionState);
        void StatusChange(const QString &);

    };
}

#endif // incl_Communication_SessionManager_h