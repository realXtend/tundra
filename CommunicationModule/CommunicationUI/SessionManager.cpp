// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SessionManager.h"

#include <UiModule.h>
#include <UiProxyWidget.h>
#include "MasterWidget.h"
#include "SessionHelper.h"
#include "FriendListWidget.h"
#include "ChatSessionWidget.h"
#include "ModuleManager.h"
#include "UiSceneManager.h"
#include "ConnectionInterface.h"
#include "ChatSessionInterface.h"
#include "Framework.h"

#include <QDebug>
#include <QMessageBox>

#include "MemoryLeakCheck.h"

namespace UiManagers
{
    SessionManager::SessionManager(CommunicationUI::MasterWidget *parent, Foundation::Framework *framework)
        : QObject(),
          main_parent_(parent),
          session_manager_ui_(0),
          session_helper_(0),
          friend_list_widget_(0),
          im_connection_(0),
          menu_bar_(0),
          spatial_voice_manager_widget_(0),
          framework_(framework)
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

    void SessionManager::Start(const QString &username, Communication::ConnectionInterface *im_connection, CommunicationUI::EventHandler *event_handler)
    {
        // Init internals
        assert(session_manager_ui_);
        session_manager_ui_->inputFrame->hide();

        session_helper_ = new UiHelpers::SessionHelper(main_parent_, im_connection, event_handler);
        session_helper_->SetupManagerUi(session_manager_ui_);
        event_handler_ = event_handler;

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

        // Connect input system cancel button
        connect(session_manager_ui_->inputQuestionButtonCancel, SIGNAL( clicked() ),
                session_manager_ui_->inputFrame, SLOT( hide() ));
        // Connect slots to ConnectionInterface
        connect(im_connection_, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface& ) ), SLOT( ChatSessionReceived(Communication::ChatSessionInterface&) ));
        connect(im_connection_, SIGNAL( VoiceSessionReceived(Communication::VoiceSessionInterface&) ), SLOT( VideoSessionReceived(Communication::VoiceSessionInterface&) ));
    }

    void SessionManager::HideFriendListWidget()
    {
        if (friend_list_widget_) 
            return friend_list_widget_->hide();
    }

    QMenuBar *SessionManager::ConstructMenuBar()
    {
        // QMenuBar and QMenu init
        menu_bar_ = new QMenuBar(main_parent_);
        
        // FILE MENU
        QMenu *file_menu = new QMenu("File", main_parent_);
        QAction *hide_action = file_menu->addAction("Hide", this, SLOT( Hide() ));
        //hide_action->setEnabled(false); // Can't support hide on external mode

        // STATUS menu
        QMenu *status_menu = new QMenu("Status", main_parent_);
       
        set_status_message = status_menu->addAction("Set Status Message", session_helper_, SLOT( SetStatusMessage() ));
        status_menu->addSeparator();
        available_status = status_menu->addAction("Available", this, SLOT( StatusAvailable() ));
        chatty_status = status_menu->addAction("Chatty", this, SLOT( StatusChatty() ));
        away_status = status_menu->addAction("Away", this, SLOT( StatusAway() ));
        extended_away_status = status_menu->addAction("Extended Away", this, SLOT( StatusExtendedAway() ));
        busy_status = status_menu->addAction("Busy", this, SLOT( StatusBusy() ));
        hidden_status = status_menu->addAction("Hidden", this, SLOT( StatusHidden() ));

        set_status_message->setIcon(QIcon(":images/iconRename.png"));
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
        signout = status_menu->addAction("Sign out", this, SLOT( SignOut() ));
        signout->setIcon(QIcon(":images/iconSignout.png"));
        
        // JOIN MENU
        QMenu *actions_menu = new QMenu("Actions", main_parent_);
        add_new_friend = actions_menu->addAction("Add New Friend");
        add_new_friend->setIcon(QIcon(":images/iconAdd.png"));
        join_chat_room = actions_menu->addAction("Join Chat Room", this, SLOT( JoinChatRoom() ));
        join_chat_room->setIcon(QIcon(":/images/iconConference.png"));
        manage_spatial_voice = actions_menu->addAction("Manage 3D Voice", this, SLOT( Show3DSoundManager() ));
        manage_spatial_voice->setIcon(QIcon(":images/iconProperties.png"));
        
        // Add sub menus to menu bar
        menu_bar_->addMenu(file_menu);
        menu_bar_->addMenu(status_menu);
        menu_bar_->addMenu(actions_menu);
        menu_bar_->addAction("Show Friend List", this, SLOT( ToggleShowFriendList() ));
        
        // Connect signals
        connect(add_new_friend, SIGNAL( triggered() ),
                session_helper_, SLOT( SendFriendRequest() ));
        connect(this, SIGNAL( StatusChange(const QString&) ), 
                session_helper_, SLOT( SetMyStatus(const QString&) ));
        connect(session_helper_, SIGNAL( ChangeMenuBarStatus(const QString &) ), 
                this, SLOT( StatusChangedOutSideMenuBar(const QString &) ));

        return menu_bar_;
    }

    void SessionManager::CreateFriendListWidget()
    {
        SAFE_DELETE(friend_list_widget_);
        friend_list_widget_ = new CommunicationUI::FriendListWidget(im_connection_, session_helper_, framework_);

        // Add friend list to scene, no toolbar button
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (ui_module.get())
        {
            UiServices::UiWidgetProperties widget_properties("Friends List", UiServices::SceneWidget);
            ui_module->GetSceneManager()->AddWidgetToScene(friend_list_widget_, widget_properties);
        }
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
        session_manager_ui_->inputQuestionLabel->setText("Give chat room name");
        session_manager_ui_->inputQuestionLineEdit->clear();
        session_manager_ui_->inputFrame->show();

        session_manager_ui_->inputQuestionButtonOk->disconnect(SIGNAL(clicked()));
        session_manager_ui_->inputQuestionLineEdit->disconnect(SIGNAL(returnPressed()));
        connect(session_manager_ui_->inputQuestionButtonOk, SIGNAL( clicked() ),
                this, SLOT( JoinChatRoomInputGiven() ));
        connect(session_manager_ui_->inputQuestionLineEdit, SIGNAL( returnPressed() ),
                this, SLOT( JoinChatRoomInputGiven() ));
    }

    void SessionManager::JoinChatRoomInputGiven()
    {
        QString chat_room_name = session_manager_ui_->inputQuestionLineEdit->text();
        if (!chat_room_name.isEmpty())
        {
            Communication::ChatSessionInterface *chat_session = im_connection_->OpenChatSession(chat_room_name);
            session_helper_->CreateNewChatSessionWidget(chat_session, chat_room_name);
            session_manager_ui_->inputFrame->hide();
        }
        else
            session_manager_ui_->inputQuestionLineEdit->setText("Input can't be empty");
    }

    void SessionManager::SignOut()
    {
        im_connection_->Close();          
        friend_list_widget_->close(); 
        SAFE_DELETE(friend_list_widget_);
        emit StateChange(UiDefines::UiStates::Disconnected);
    }

    void SessionManager::Show3DSoundManager()
    {
        if (event_handler_)
        {
            avatar_map_ = event_handler_->GetAvatarList();
            if (!avatar_map_.empty())
            {
                SAFE_DELETE(spatial_voice_manager_widget_);
                spatial_voice_manager_widget_ = new QWidget(0);
                spatial_voice_configure_ui_.setupUi(spatial_voice_manager_widget_);

                foreach (QString name, avatar_map_.values())
                    spatial_voice_configure_ui_.avatarsComboBox->addItem(name);

                connect(spatial_voice_configure_ui_.updateAvatartPushButton, SIGNAL( clicked() ), SLOT( UpdateAvatarList() ));
                connect(spatial_voice_configure_ui_.startTrackingPushButton, SIGNAL( clicked() ), SLOT( StartTrackingSelectedAvatar() ));
                connect(spatial_voice_configure_ui_.stopTrackingPushButton, SIGNAL( clicked() ), SLOT( StopTrackingSelectedAvatar() ));

                spatial_voice_manager_widget_->show();
            }
            else
                QMessageBox::information(0, "Manager Error", "Could not update avatars from world, are you sure you are connected to a server?");
        }
    }

    void SessionManager::Hide()
    {
        main_parent_->hide();
    }

    void SessionManager::StatusAvailable()
    {
        emit StatusChange(QString("available"));
    }
    void SessionManager::StatusChatty()
    {
        emit StatusChange(QString("chat"));
    }
    void SessionManager::StatusAway()
    { 
        emit StatusChange(QString("away"));
    }
    void SessionManager::StatusExtendedAway()
    { 
        emit StatusChange(QString("xa"));
    }
    void SessionManager::StatusBusy()
    { 
        emit StatusChange(QString("dnd"));
    }
    void SessionManager::StatusHidden()
    {
        emit StatusChange(QString("hidden"));
    }

    void SessionManager::ToggleShowFriendList()
    { 
        if (friend_list_widget_->isVisible()) 
            friend_list_widget_->hide(); 
        else 
            friend_list_widget_->show();
    } 

    void SessionManager::UpdateAvatarList()
    {
        if (!spatial_voice_manager_widget_)
            return;

        avatar_map_ = event_handler_->GetAvatarList();
        if (!avatar_map_.empty())
        {
            spatial_voice_configure_ui_.avatarsComboBox->clear();
            foreach (QString name, avatar_map_.values())
                spatial_voice_configure_ui_.avatarsComboBox->addItem(name);
        }
    }

    void SessionManager::StartTrackingSelectedAvatar()
    {
        QString selected_avatar_name = spatial_voice_configure_ui_.avatarsComboBox->currentText();
        QString avatar_id = avatar_map_.key(selected_avatar_name);
        if (!avatar_id.isEmpty() || !avatar_id.isNull())
            event_handler_->StartTrackingSelectedAvatar(avatar_id);
    }

    void SessionManager::StopTrackingSelectedAvatar()
    {
        event_handler_->StopTrackingSelectedAvatar();
    }
}
