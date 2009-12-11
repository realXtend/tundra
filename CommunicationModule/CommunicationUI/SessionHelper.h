// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_SessionHelper_h
#define incl_Communication_SessionHelper_h

#include "Foundation.h"
#include "ui_SessionManagerWidget.h"
#include <QObject>

#include "interface.h"

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
        SessionHelper(QWidget *main_parent, Communication::ConnectionInterface  *im_connection);
        virtual ~SessionHelper();

        //! UI Setter
        void SetupManagerUi(Ui::SessionManagerWidget *session_manager_ui) { SAFE_DELETE(session_manager_ui_); session_manager_ui_ = session_manager_ui; }
        void SetupFriendListUi(CommunicationUI::FriendListWidget *friend_list_widget) { friend_list_widget_ = friend_list_widget; }

    public slots:
        //! Setters
        void SetOwnName(const QString &my_name) { my_name_ = my_name; }
        void SetStatusMessage();
        void SetMyStatus(const QString &status_code);
        void SetMyStatusMessage(const QString &status_message);
        
        //! Getters
        QString GetFriendsNameFromParticipants(Communication::ChatSessionParticipantVector participant_vector);
        QString &GetPresenceStatus() { return presence_status_; }

        //! Checkers
        bool DoesTabExist(const QString &chat_friends_name);

        //! Doers
        void CloseTab(const QString &chat_friends_name);
        void CreateNewChatSessionWidget(Communication::ChatSessionInterface *chat_session, QString &chat_friends_name);

    private:
        Ui::SessionManagerWidget            *session_manager_ui_;
        CommunicationUI::FriendListWidget   *friend_list_widget_;
        Communication::ConnectionInterface  *im_connection_;

        QWidget *main_parent_;
        QWidget *info_widget_;
        QString presence_status_;
        QString presence_status_message_;
        QString my_name_;
        bool welcome_tab_destroyed;

    signals:
        void ChangeMenuBarStatus(const QString &);

    };
}

#endif // incl_Communication_SessionHelper_h