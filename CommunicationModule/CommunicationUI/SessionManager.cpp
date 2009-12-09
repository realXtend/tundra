// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SessionManager.h"

#include <QMenuBar>

namespace UiHelpers
{
    SessionManager::SessionManager()
        : QObject(),
          session_manager_ui_(0),
          im_connection_(0),
          menu_bar_(0)
    {

    }

    SessionManager::~SessionManager()
    {
        SAFE_DELETE(menu_bar_);
    }

    void SessionManager::Start(const QString &username, Communication::ConnectionInterface *im_connection)
    {
        // Init internals
        assert(session_manager_ui_);
        im_connection_ = im_connection;

        // Init ui
        menu_bar_ = new QMenuBar();
        QMenu *menu = new QMenu("File");
        menu->addMenu("Logout");
        menu->addMenu("Close");
        menu_bar_->addMenu(menu);
        session_manager_ui_->mainVerticalLayout->insertWidget(0, menu_bar_);
        session_manager_ui_->usernameLabel->setText(username);

        // Connect slots to ConnectionInterface
        /*
        connect(im_connection_, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface& ) ), this, SLOT( NewChatSessionRequest(Communication::ChatSessionInterface&) ));
	    connect(im_connection_, SIGNAL( FriendRequestReceived(Communication::FriendRequestInterface&) ), this, SLOT( NewFriendRequest(Communication::FriendRequestInterface&) ));
        connect(im_connection_, SIGNAL( NewContact(const Communication::ContactInterface&) ), this, SLOT( OnNewContact(const Communication::ContactInterface&) ));
        connect(im_connection_, SIGNAL( ContactRemoved(const Communication::ContactInterface&) ), this, SLOT( OnContactRemoved(const Communication::ContactInterface&) ));
        */

        //CreateFriendsManager()
    }
}