// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SessionHelper.h"
#include "FriendListWidget.h"
#include "ChatSessionWidget.h"
#include "VideoSessionWidget.h"
#include "UiDefines.h"
#include "EventHandler.h"
#include "ConnectionInterface.h"
#include "ChatSessionParticipantInterface.h"
#include "VoiceSessionParticipantInterface.h"

#include <QDebug>
#include <QPair>
#include <QPixmap>

#include "MemoryLeakCheck.h"

namespace UiHelpers
{
    SessionHelper::SessionHelper(QWidget *main_parent, Communication::ConnectionInterface  *im_connection, CommunicationUI::EventHandler *event_handler)
        : QObject(),
          main_parent_(main_parent),
          im_connection_(im_connection),
          event_handler_(event_handler),
          session_manager_ui_(0),
          friend_list_widget_(0),
          info_widget_(0),
          presence_status_(""),
          presence_status_message_(""),
          welcome_tab_destroyed(false)
    {

    }

    SessionHelper::~SessionHelper()
    {

    }

    /************* SET IT *************/

    void SessionHelper::SetStatusMessage()
    {
        session_manager_ui_->inputQuestionLabel->setText("New Status Message");
        session_manager_ui_->inputQuestionLineEdit->clear();
        session_manager_ui_->inputFrame->show();

        session_manager_ui_->inputQuestionButtonOk->disconnect(SIGNAL(clicked()));
        session_manager_ui_->inputQuestionLineEdit->disconnect(SIGNAL(returnPressed()));
        connect(session_manager_ui_->inputQuestionButtonOk, SIGNAL( clicked() ),
                this, SLOT( StatusMessageInputGiven() ));
        connect(session_manager_ui_->inputQuestionLineEdit, SIGNAL( returnPressed() ),
                this, SLOT( StatusMessageInputGiven() ));
    }

    void SessionHelper::StatusMessageInputGiven()
    {
        SetMyStatusMessage(session_manager_ui_->inputQuestionLineEdit->text());
        session_manager_ui_->inputFrame->hide();
    }

    void SessionHelper::SetMyStatus(const QString &status_code)
    {
        if (status_code != presence_status_)
        {
            im_connection_->SetPresenceStatus(status_code);
            presence_status_ = status_code;
            QPixmap status_pixmap(UiDefines::PresenceStatus::GetImagePathForStatusCode(status_code));
            session_manager_ui_->statusIconLabel->setPixmap(status_pixmap.scaled(24, 24, Qt::KeepAspectRatio));

            if (UiDefines::PresenceStatus::GetStatusCodeForStatus(friend_list_widget_->GetStatus()) != status_code)
                friend_list_widget_->SetStatus(status_code);

            emit ChangeMenuBarStatus(status_code);
        }
    }

    void SessionHelper::SetMyStatusMessage(const QString &status_message)
    {
        im_connection_->SetPresenceMessage(status_message);
        presence_status_message_ = status_message;
        session_manager_ui_->statusMessageLabel->setText(status_message);
    }

    /************* GET IT *************/

    QString SessionHelper::GetChatInviteSendersName(Communication::ChatSessionParticipantVector participant_vector)
    {
        Communication::ChatSessionParticipantVector::const_iterator iter;
        for( iter=participant_vector.begin(); iter!=participant_vector.end(); iter++ )
        {
            Communication::ChatSessionParticipantInterface *participant = (*iter);
            if (participant->GetID() != my_name_)
                return participant->GetID();
        }
        return "";
    }

    QString SessionHelper::GetVideoInviteSendersName(Communication::VoiceSessionParticipantVector participant_vector)
    {
        Communication::VoiceSessionParticipantVector::const_iterator iter;
        for( iter=participant_vector.begin(); iter!=participant_vector.end(); iter++ )
        {
            Communication::VoiceSessionParticipantInterface *participant = (*iter);
            if (participant->GetID() != my_name_)
                return participant->GetID();
        }
        return "";
    }

    /************* CHECK IT *************/

    bool SessionHelper::DoesChatTabExist(const QString &chat_friends_name)
    {
        if (chat_sessions_pointers_map_.contains(chat_friends_name))
        {
            QWidget *found_tab_widget = chat_sessions_pointers_map_[chat_friends_name].first;
            session_manager_ui_->sessionsTabWidget->setCurrentWidget(found_tab_widget);
            return true;
        }
        return false;
    }

    bool SessionHelper::DoesVideoTabExist(const QString &video_friends_name)
    {
        if (video_sessions_pointers_map_.contains(video_friends_name))
        {
            QWidget *found_tab_widget = video_sessions_pointers_map_[video_friends_name].first;
            session_manager_ui_->sessionsTabWidget->setCurrentWidget(found_tab_widget);
            return true;
        }
        return false;
    }

    void SessionHelper::TabWidgetPreStateCheck()
    {
        if (!welcome_tab_destroyed)
        {
            session_manager_ui_->sessionsTabWidget->clear();
            welcome_tab_destroyed = true;
        }

        if (info_widget_)
        {
            info_widget_->findChild<QLabel *>("infoLabel")->hide();
            SAFE_DELETE(info_widget_);
        }
    }

    void SessionHelper::TabWidgetPostStateCheck()
    {
        if (session_manager_ui_->sessionsTabWidget->count() == 0)
        {
            SAFE_DELETE(info_widget_);
            info_widget_ = new QWidget();
            info_widget_->setObjectName("infoWidget");
            info_widget_->setStyleSheet(QString("QWidget#infoWidget { background-color: rgb(255,255,255); } QLabel { color: rgb(0,0,0); }"));
            QVBoxLayout *layout = new QVBoxLayout(info_widget_);
            layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));
            QLabel *info_label = new QLabel("Click on Show Friend List from the top menu to start communicating", info_widget_);
            info_label->setObjectName("infoLabel");
            info_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            info_label->setAlignment(Qt::AlignCenter);
            layout->addWidget(info_label);
            layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));
            info_widget_->setLayout(layout);

            session_manager_ui_->sessionsTabWidget->addTab(info_widget_, "  Communications");
        }
    }

    /************* DO IT *************/

    void SessionHelper::SendFriendRequest()
    {
        session_manager_ui_->inputQuestionLabel->setText("Give new friends IM address");
        session_manager_ui_->inputQuestionLineEdit->clear();
        session_manager_ui_->inputFrame->show();

        session_manager_ui_->inputQuestionButtonOk->disconnect(SIGNAL(clicked()));
        session_manager_ui_->inputQuestionLineEdit->disconnect(SIGNAL(returnPressed()));
        connect(session_manager_ui_->inputQuestionButtonOk, SIGNAL( clicked() ),
                this, SLOT( FriendAddInputGiven() ));
        connect(session_manager_ui_->inputQuestionLineEdit, SIGNAL( returnPressed() ),
                this, SLOT( FriendAddInputGiven() ));
    }

    void SessionHelper::FriendAddInputGiven()
    {
        QString request_address = session_manager_ui_->inputQuestionLineEdit->text();
        if (!request_address.isEmpty() && request_address.count(" ") == 0 && request_address.count("@") == 1)
        {
            im_connection_->SendFriendRequest(request_address, "");
            session_manager_ui_->inputFrame->hide();
        }
        else
            session_manager_ui_->inputQuestionLineEdit->setText("Please give valid email (no spaces and has @)");
    }


    void SessionHelper::CloseChatTab(const QString &chat_friends_name)
    {
        if (chat_sessions_pointers_map_.contains(chat_friends_name))
        {
            QWidget *found_tab_widget = chat_sessions_pointers_map_[chat_friends_name].first;
            int index = session_manager_ui_->sessionsTabWidget->indexOf(found_tab_widget);
            session_manager_ui_->sessionsTabWidget->removeTab(index);
            chat_sessions_pointers_map_.remove(chat_friends_name);
    
            //SAFE_DELETE(found_tab_widget); // lets not delete this due it has a parent and will be deleted later
            TabWidgetPostStateCheck();
        }
    }

    void SessionHelper::CloseVideoTab(const QString &chat_friends_name)
    {
        if (video_sessions_pointers_map_.contains(chat_friends_name))
        {
            QWidget *found_tab_widget = video_sessions_pointers_map_[chat_friends_name].first;
            int index = session_manager_ui_->sessionsTabWidget->indexOf(found_tab_widget);
            session_manager_ui_->sessionsTabWidget->removeTab(index);
            video_sessions_pointers_map_.remove(chat_friends_name);

            //SAFE_DELETE(found_tab_widget); // lets not delete this due it has a parent and will be deleted later
            TabWidgetPostStateCheck();
        }
    }

    void SessionHelper::CreateNewChatSessionWidget(Communication::ChatSessionInterface *chat_session, QString &chat_friends_name)
    {
        TabWidgetPreStateCheck();

        if (!DoesChatTabExist(chat_friends_name))
        {
            // Add tab and connections
            CommunicationUI::ChatSessionWidget *chat_session_tab = new CommunicationUI::ChatSessionWidget(main_parent_, chat_session, my_name_, chat_friends_name);
            int index = session_manager_ui_->sessionsTabWidget->addTab(chat_session_tab, QIcon(":images/iconChat.png"), chat_friends_name);
            session_manager_ui_->sessionsTabWidget->setCurrentIndex(index);
            connect(chat_session_tab, SIGNAL( Closed(const QString &) ), this, SLOT( CloseChatTab(const QString &) ));

            // Store to local container
            chat_sessions_pointers_map_[chat_friends_name].first = chat_session_tab;
            chat_sessions_pointers_map_[chat_friends_name].second = chat_session;
        }
    }

    void SessionHelper::CreateNewVideoSessionWidget(Communication::VoiceSessionInterface *video_session, QString &chat_friends_name)
    {
        TabWidgetPreStateCheck();

        if (!DoesVideoTabExist(chat_friends_name))
        {
            // Add tab and connections
            CommunicationUI::VideoSessionWidget *video_session_tab = new CommunicationUI::VideoSessionWidget(main_parent_, video_session, my_name_, chat_friends_name);
            int index = session_manager_ui_->sessionsTabWidget->addTab(video_session_tab, QIcon(":images/iconVideo.png"), chat_friends_name);
            session_manager_ui_->sessionsTabWidget->setCurrentIndex(index);
            connect(video_session_tab, SIGNAL( Closed(const QString &) ), this, SLOT( CloseVideoTab(const QString &) ));

            // Store to local container
            video_sessions_pointers_map_[chat_friends_name].first = video_session_tab;
            video_sessions_pointers_map_[chat_friends_name].second = video_session;

            connect(event_handler_, SIGNAL( AvatarPositionUpdated(Vector3df) ), video_session, SLOT( UpdateAudioSourcePosition(Vector3df) ));
            connect(event_handler_, SIGNAL( TrackingAvatar(bool) ), video_session, SLOT( TrackingAvatar(bool) ));
        }
    }
}