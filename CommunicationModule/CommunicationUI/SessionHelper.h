// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_SessionHelper_h
#define incl_Communication_SessionHelper_h

//#include "Foundation.h"
#include "ui_SessionManagerWidget.h"

#include "ChatSessionWidget.h"
#include "VideoSessionWidget.h"

//#include "EventHandler.h"

#include <QObject>
#include <QMap>
#include <QPair>

//#include "interface.h"

#include "CommunicationModuleFwd.h"

namespace CommunicationUI
{
    class FriendListWidget;
}

namespace UiHelpers
{
    class SessionHelper : public QObject
    {

    Q_OBJECT
    Q_PROPERTY(QString presence_status_ READ GetPresenceStatus)

    public:
        SessionHelper(QWidget *main_parent, Communication::ConnectionInterface *im_connection, CommunicationUI::EventHandler *event_handler);
        virtual ~SessionHelper();

        //! UI Setter
        void SetupManagerUi(Ui::SessionManagerWidget *session_manager_ui) { SAFE_DELETE(session_manager_ui_); session_manager_ui_ = session_manager_ui; }
        void SetupFriendListUi(CommunicationUI::FriendListWidget *friend_list_widget) { friend_list_widget_ = friend_list_widget; }

    public slots:
        //! Setters
        void SetOwnName(const QString &my_name) { my_name_ = my_name; }
        void SetStatusMessage();
        void StatusMessageInputGiven();
        void SetMyStatus(const QString &status_code);
        void SetMyStatusMessage(const QString &status_message);
        
        //! Getters
        QString GetChatInviteSendersName(Communication::ChatSessionParticipantVector participant_vector);
        QString GetVideoInviteSendersName(Communication::VoiceSessionParticipantVector participant_vector);
        QString &GetPresenceStatus() { return presence_status_; }

        //! Checkers
        bool DoesChatTabExist(const QString &chat_friends_name);
        bool DoesVideoTabExist(const QString &video_friends_name);
        void TabWidgetPreStateCheck();
        void TabWidgetPostStateCheck();

        //! Doers
        void SendFriendRequest();
        void FriendAddInputGiven();
        void CloseChatTab(const QString &chat_friends_name);
        void CloseVideoTab(const QString &chat_friends_name);
        void CreateNewChatSessionWidget(Communication::ChatSessionInterface *chat_session, QString &chat_friends_name);
        void CreateNewVideoSessionWidget(Communication::VoiceSessionInterface *video_session, QString &chat_friends_name);

    private:
        Ui::SessionManagerWidget            *session_manager_ui_;
        CommunicationUI::FriendListWidget   *friend_list_widget_;
        CommunicationUI::EventHandler       *event_handler_;
        Communication::ConnectionInterface  *im_connection_;

        QWidget *main_parent_;
        QWidget *info_widget_;
        QString presence_status_;
        QString presence_status_message_;
        QString my_name_;
        bool welcome_tab_destroyed;

        QMap<QString, QPair<CommunicationUI::ChatSessionWidget *, Communication::ChatSessionInterface *> > chat_sessions_pointers_map_;
        QMap<QString, QPair<CommunicationUI::VideoSessionWidget *, Communication::VoiceSessionInterface *> > video_sessions_pointers_map_;

    signals:
        void ChangeMenuBarStatus(const QString &);

    };
}

#endif // incl_Communication_SessionHelper_h