// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SessionManager.h"

#include <QDebug>

namespace UiHelpers
{
    SessionManager::SessionManager(CommunicationUI::MasterWidget *parent)
        : QObject(),
          parent_(parent),
          presence_status_(UiDefines::PresenceStatus::NoStatus),
          session_manager_ui_(0),
          im_connection_(0),
          menu_bar_(0)
    {

    }

    SessionManager::~SessionManager()
    {
        SAFE_DELETE(menu_bar_);
        parent_ = 0;
        session_manager_ui_ = 0;
        im_connection_ = 0;
    }

    void SessionManager::Start(const QString &username, Communication::ConnectionInterface *im_connection)
    {
        // Init internals
        assert(session_manager_ui_);
        im_connection_ = im_connection;

        session_manager_ui_->mainVerticalLayout->insertWidget(0, ConstructMenuBar());
        session_manager_ui_->usernameLabel->setText(username);

        // Connect slots to ConnectionInterface
        connect(im_connection_, SIGNAL( VoiceSessionReceived(Communication::VoiceSessionInterface&) ), SLOT( VoiceSessionRecieved(Communication::VoiceSessionInterface&) ));
        /*
        connect(im_connection_, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface& ) ), this, SLOT( NewChatSessionRequest(Communication::ChatSessionInterface&) ));
	    connect(im_connection_, SIGNAL( FriendRequestReceived(Communication::FriendRequestInterface&) ), this, SLOT( NewFriendRequest(Communication::FriendRequestInterface&) ));
        connect(im_connection_, SIGNAL( NewContact(const Communication::ContactInterface&) ), this, SLOT( OnNewContact(const Communication::ContactInterface&) ));
        connect(im_connection_, SIGNAL( ContactRemoved(const Communication::ContactInterface&) ), this, SLOT( OnContactRemoved(const Communication::ContactInterface&) ));
        */

        //CreateFriendsManager()
        
        StatusHandler(UiDefines::PresenceStatus::Available);

        qDebug() << "\nFriendlist:\n" << endl;
        Communication::ContactVector contacts = im_connection_->GetContacts().GetContacts();
        Communication::ContactVector::const_iterator iter;

        for( iter=contacts.begin(); iter!=contacts.end(); iter++ )
		{
            Communication::ContactInterface *contact = (*iter);
            qDebug() << contact->GetName() << "(" << contact->GetPresenceStatus() << ")" << endl;
        }
    }

    QMenuBar *SessionManager::ConstructMenuBar()
    {
        // QMenuBar and QMenu init
        menu_bar_ = new QMenuBar(parent_);
        
        // FILE menu
        QMenu *file_menu = new QMenu("File", parent_);    
        
        // FILE -> CHANGE STATUS menu
        QMenu *status_menu = new QMenu("Change Status", parent_);
        
        QAction *available_status = status_menu->addAction("Available", this, SLOT( StatusAvailable() ));
        QAction *away_status = status_menu->addAction("Away", this, SLOT( StatusAway() ));
        QAction *busy_status = status_menu->addAction("Busy", this, SLOT( StatusBusy() ));
        QAction *offline_status = status_menu->addAction("Show as offline", this, SLOT( StatusOffline() ));
        
        available_status->setCheckable(true);
        away_status->setCheckable(true);
        busy_status->setCheckable(true);
        offline_status->setCheckable(true);

        QActionGroup *status_group = new QActionGroup(parent_);
        status_group->addAction(available_status);
        status_group->addAction(away_status);
        status_group->addAction(busy_status);
        status_group->addAction(offline_status);
        available_status->setChecked(true);

        // FILE content
        file_menu->addMenu(status_menu);
        file_menu->addAction("Sign out", this, SLOT( SignOut() ));
        file_menu->addSeparator();
        file_menu->addAction("Hide", this, SLOT( Hide() ));
        file_menu->addAction("Exit", this, SLOT( Exit() ));
        menu_bar_->addMenu(file_menu);

        connect(this, SIGNAL( StatusChange(UiDefines::PresenceStatus::PresenceState) ), SLOT( StatusHandler(UiDefines::PresenceStatus::PresenceState) ));
        return menu_bar_;
    }

    void SessionManager::StatusHandler(UiDefines::PresenceStatus::PresenceState new_status)
    {
        if (GetPresenceStatus() != new_status)
        {
            QString status;

            switch (new_status)
            {
                case UiDefines::PresenceStatus::Available:
                    status = "Available";
                    break;
                case UiDefines::PresenceStatus::Away:
                    status = "Away";
                    break;
                case UiDefines::PresenceStatus::Busy:
                    status = "Busy";
                    break;
                case UiDefines::PresenceStatus::Offline:
                    status = "Offline";
                    break;
            }

            im_connection_->SetPresenceStatus(status);
        }
    }

    void SessionManager::VoiceSessionRecieved(Communication::VoiceSessionInterface& voice_session)
    {
        qDebug() << "Voice session recieved with state" << QString::number((int)voice_session.GetState()) << endl;
        voice_session.Accept();
    }
}