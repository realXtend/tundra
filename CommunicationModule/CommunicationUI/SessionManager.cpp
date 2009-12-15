// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SessionManager.h"

#include <QDebug>
#include <QInputDialog>

namespace UiManagers
{
    SessionManager::SessionManager(CommunicationUI::MasterWidget *parent)
        : QObject(),
          main_parent_(parent),
          session_manager_ui_(0),
          session_helper_(0),
          friend_list_widget_(0),
          im_connection_(0),
          menu_bar_(0)
    {

    }

    SessionManager::~SessionManager()
    {
        SAFE_DELETE(session_helper_);
        SAFE_DELETE(friend_list_widget_);
        main_parent_ = 0;
        session_manager_ui_ = 0;
        im_connection_ = 0;
    }

    void SessionManager::Start(const QString &username, Communication::ConnectionInterface *im_connection)
    {
        // Init internals
        assert(session_manager_ui_);
        session_helper_ = new UiHelpers::SessionHelper(main_parent_, im_connection);
        session_helper_->SetupManagerUi(session_manager_ui_);

        im_connection_ = im_connection;
        im_connection_->SetPresenceStatus(session_helper_->GetPresenceStatus());

        session_manager_ui_->mainVerticalLayout->insertWidget(0, ConstructMenuBar());
        session_manager_ui_->usernameLabel->setText(username);
        session_helper_->SetOwnName(username);

        // Set own status and create friend list widget
        CreateFriendListWidget();
        session_helper_->SetupFriendListUi(friend_list_widget_);
        session_helper_->SetMyStatus("available");
        session_helper_->SetMyStatusMessage(im_connection_->GetPresenceMessage());

        // Connect slots to ConnectionInterface
        connect(im_connection_, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface& ) ), SLOT( ChatSessionReceived(Communication::ChatSessionInterface&) ));
        connect(im_connection_, SIGNAL( VoiceSessionReceived(Communication::VoiceSessionInterface&) ), SLOT( VideoSessionReceived(Communication::VoiceSessionInterface&) ));
    }

    QMenuBar *SessionManager::ConstructMenuBar()
    {
        // QMenuBar and QMenu init
        menu_bar_ = new QMenuBar(main_parent_);
        
        // STATUS menu
        QMenu *status_menu = new QMenu("Status", main_parent_);
       
        status_menu->addAction("Set Status Message", session_helper_, SLOT( SetStatusMessage() ));
        status_menu->addSeparator();
        available_status = status_menu->addAction("Available", this, SLOT( StatusAvailable() ));
        chatty_status = status_menu->addAction("Chatty", this, SLOT( StatusChatty() ));
        away_status = status_menu->addAction("Away", this, SLOT( StatusAway() ));
        extended_away_status = status_menu->addAction("Extended Away", this, SLOT( StatusExtendedAway() ));
        busy_status = status_menu->addAction("Busy", this, SLOT( StatusBusy() ));
        hidden_status = status_menu->addAction("Hidden", this, SLOT( StatusHidden() ));
        
        available_status->setCheckable(true);
        available_status->setIcon(UiDefines::PresenceStatus::GetIconForStatusCode("available"));
        chatty_status->setCheckable(true);
        chatty_status->setIcon(UiDefines::PresenceStatus::GetIconForStatusCode("chat"));
        away_status->setCheckable(true);
        away_status->setIcon(UiDefines::PresenceStatus::GetIconForStatusCode("away"));
        extended_away_status->setCheckable(true);
        extended_away_status->setIcon(UiDefines::PresenceStatus::GetIconForStatusCode("xa"));
        busy_status->setCheckable(true);
        busy_status->setIcon(UiDefines::PresenceStatus::GetIconForStatusCode("dnd"));
        hidden_status->setCheckable(true);
        hidden_status->setIcon(UiDefines::PresenceStatus::GetIconForStatusCode("hidden"));

        QActionGroup *status_group = new QActionGroup(main_parent_);            
        status_group->addAction(available_status);
        status_group->addAction(chatty_status);
        status_group->addAction(away_status);
        status_group->addAction(extended_away_status);
        status_group->addAction(busy_status);
        status_group->addAction(hidden_status);
        available_status->setChecked(true);

        status_menu->addSeparator();
        status_menu->addAction("Sign out", this, SLOT( SignOut() ));

        // FILE MENU
        QMenu *file_menu = new QMenu("File", main_parent_);
        file_menu->addAction("Hide", this, SLOT( Hide() ));
        file_menu->addAction("Exit", this, SLOT( Exit() ));
        
        // JOIN MENU
        QMenu *join_menu = new QMenu("Join", main_parent_);
        QAction *join_chat_room = join_menu->addAction("Chat Room", this, SLOT( JoinChatRoom() ));
        join_chat_room->setIcon(QIcon(":/images/iconUsers.png"));
        
        menu_bar_->addMenu(file_menu);
        menu_bar_->addMenu(status_menu);
        menu_bar_->addMenu(join_menu);
        menu_bar_->addAction("Show Friend List", this, SLOT( ToggleShowFriendList() ));

        connect(this, SIGNAL( StatusChange(const QString&) ), 
                session_helper_, SLOT( SetMyStatus(const QString&) ));
        connect(session_helper_, SIGNAL( ChangeMenuBarStatus(const QString &) ), 
                this, SLOT( StatusChangedOutSideMenuBar(const QString &) ));

        return menu_bar_;
    }

    void SessionManager::CreateFriendListWidget()
    {
        SAFE_DELETE(friend_list_widget_);
        friend_list_widget_ = new CommunicationUI::FriendListWidget(im_connection_, session_helper_);

        connect(friend_list_widget_, SIGNAL( StatusChanged(const QString &) ),
                session_helper_, SLOT( SetMyStatus(const QString &) ));
        connect(friend_list_widget_, SIGNAL( NewChatSessionStarted(Communication::ChatSessionInterface *, QString &) ),
                session_helper_, SLOT( CreateNewChatSessionWidget(Communication::ChatSessionInterface *, QString &) ));
        connect(friend_list_widget_, SIGNAL( NewVideoSessionStarted(Communication::VoiceSessionInterface *, QString &) ),
                session_helper_, SLOT( CreateNewVideoSessionWidget(Communication::VoiceSessionInterface *, QString &) ));
    }

    void SessionManager::StatusChangedOutSideMenuBar(const QString &status_code)
    {
        if (status_code == "available")
            available_status->setChecked(true);
        else if (status_code == "chat")
            chatty_status->setChecked(true);
        else if (status_code == "away")
            away_status->setChecked(true);
        else if (status_code == "xa")
            extended_away_status->setChecked(true);
        else if (status_code == "dnd")
            busy_status->setChecked(true);
        else if (status_code == "hidden")
            hidden_status->setChecked(true);
    }

    void SessionManager::ChatSessionReceived(Communication::ChatSessionInterface &chat_session)
    {
        QString friends_name = session_helper_->GetChatInviteSendersName(chat_session.GetParticipants());
        session_helper_->CreateNewChatSessionWidget(static_cast<Communication::ChatSessionInterface *>(&chat_session), friends_name);
    }

    void SessionManager::VideoSessionReceived(Communication::VoiceSessionInterface &video_session)
    {
        QString friends_name = session_helper_->GetVideoInviteSendersName(video_session.GetParticipants());
        session_helper_->CreateNewVideoSessionWidget(static_cast<Communication::VoiceSessionInterface *>(&video_session), friends_name);
    }

    void SessionManager::JoinChatRoom()
    {
        bool ok = false;
        QString chat_room_name = QInputDialog::getText(0, "Join Chat Room", "Give chat room name (usually room@conference.server)", QLineEdit::Normal, "", &ok);
        if (ok)
        {
            Communication::ChatSessionInterface *chat_session = im_connection_->OpenChatSession(chat_room_name);
            session_helper_->CreateNewChatSessionWidget(chat_session, chat_room_name);
        }
    }

    void SessionManager::Exit()
    {
        friend_list_widget_->close(); 
        im_connection_->Close(); 
        emit StateChange(UiDefines::UiStates::Exit);
    }
}
