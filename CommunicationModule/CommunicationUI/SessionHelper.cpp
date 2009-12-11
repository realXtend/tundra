// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SessionHelper.h"
#include "FriendListWidget.h"
#include "ChatSessionWidget.h"
#include "UiDefines.h"

#include <QDebug>
#include <QPixmap>
#include <QInputDialog>

namespace UiHelpers
{
    SessionHelper::SessionHelper(QWidget *main_parent, Communication::ConnectionInterface  *im_connection)
        : QObject(),
          main_parent_(main_parent),
          im_connection_(im_connection),
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
        bool ok = false;
        QString new_status_message = QInputDialog::getText(0, "New Status Message", "Give your new status message", QLineEdit::Normal, "", &ok);
        if (ok)
            SetMyStatusMessage(new_status_message);
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

    QString SessionHelper::GetFriendsNameFromParticipants(Communication::ChatSessionParticipantVector &participant_vector)
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

    /************* CHECK IT *************/

    bool SessionHelper::DoesTabExist(const QString &chat_friends_name)
	{
        int tab_count = session_manager_ui_->sessionsTabWidget->count();
		for (int index = 0; index < tab_count; ++index)
		{
			if ( QString::compare(session_manager_ui_->sessionsTabWidget->tabText(index), chat_friends_name) == 0 )
			{
				session_manager_ui_->sessionsTabWidget->setCurrentIndex(index);
				return true;
			}
		}
		return false;
	}

    /************* DO IT *************/

    void SessionHelper::CloseTab(const QString &chat_friends_name)
    {
        int tab_count = session_manager_ui_->sessionsTabWidget->count();
		for (int index = 0; index < tab_count; ++index)
			if ( QString::compare(session_manager_ui_->sessionsTabWidget->tabText(index), chat_friends_name) == 0 )
				session_manager_ui_->sessionsTabWidget->removeTab(index);

        // If that was the last tab, lets put some content so it wont look stupid
        if (session_manager_ui_->sessionsTabWidget->count() == 0)
        {
            SAFE_DELETE(info_widget_);
            info_widget_ = new QWidget();
            info_widget_->setObjectName("infoWidget");
            info_widget_->setStyleSheet(QString("QWidget#infoWidget { background-color: rgb(255,255,255); } QLabel { color: rgb(0,0,0); }"));
            QVBoxLayout *layout = new QVBoxLayout(info_widget_);
            layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));
            layout->addWidget(new QLabel("Click on Show Friend List from the top menu to start communicating...", info_widget_));
            layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Preferred, QSizePolicy::Expanding));
            info_widget_->setLayout(layout);

            session_manager_ui_->sessionsTabWidget->addTab(info_widget_, "  Communications");
        }
    }

    void SessionHelper::CreateNewChatSessionWidget(Communication::ChatSessionInterface *chat_session, QString &chat_friends_name)
    {
        if (!welcome_tab_destroyed)
        {
            session_manager_ui_->sessionsTabWidget->clear();
            welcome_tab_destroyed = true;
        }

        if (info_widget_)
            SAFE_DELETE(info_widget_);

        if (!DoesTabExist(chat_friends_name))
        {
            CommunicationUI::ChatSessionWidget *chat_session_tab = new CommunicationUI::ChatSessionWidget(main_parent_, chat_session, my_name_, chat_friends_name);
            int index = session_manager_ui_->sessionsTabWidget->addTab(chat_session_tab, QIcon(":images/iconChat.png"), chat_friends_name);
            session_manager_ui_->sessionsTabWidget->setCurrentIndex(index);
            connect(chat_session_tab, SIGNAL( Closed(const QString &) ), this, SLOT( CloseTab(const QString &) ));
        }
    }
}