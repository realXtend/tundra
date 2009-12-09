
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "LoginHelper.h"

#include "Credentials.h"
#include "CommunicationService.h"

#include <QUrl>

namespace UiHelpers
{
    LoginHelper::LoginHelper(Ui::LoginWidget login_ui)
        : login_ui_(login_ui),
          communication_service_(Communication::CommunicationService::GetInstance())
    {

    }

    LoginHelper::~LoginHelper()
    {

    }

    void LoginHelper::TryLogin()
    {
        QString username(login_ui_.usernameLineEdit->text());
        QString server(login_ui_.serverLineEdit->text());

        // Username validation
        if (!username.contains("@"))
	        username.append(QString("@%1").arg(server));

        // Server and port validation
        if (server.startsWith("http://") || server.startsWith("https://"))
            server = "http://" + server;
        QUrl server_url(server);
        if (server_url.port() == -1)
            server_url.setPort(5222);

        // Create the credentials
        Communication::Credentials credentials;
        credentials.SetProtocol("jabber");
        credentials.SetUserID(username);
        credentials.SetPassword(login_ui_.passwordLineEdit->text());
        credentials.SetServer(server_url.authority());
        credentials.SetPort(server_url.port());

	    try
        {
            im_connection_ = communication_service_->OpenConnection(credentials);
        }
        catch (Core::Exception &e) { /* e.what() for error */ }

        QObject::connect(im_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( ConnectionEstablished(Communication::ConnectionInterface&) ));
        QObject::connect(im_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( ConnectionFailed(Communication::ConnectionInterface&) ));
        
        emit( StateChange(UiDefines::UiStates::Connecting) );
    }

    void LoginHelper::LoginCancelled()
    {
        im_connection_->Close();
        emit( StateChange(UiDefines::UiStates::Disconnected) );
    }

    void LoginHelper::ConnectionEstablished(Communication::ConnectionInterface &connection_interface)
    {
        // Save current credentials

        // Change MasterWidgets state to connected

        // Emit a switch context with im_connection_
        // where connecting these signals to the new gui
        /*
	    connect(im_connection_, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface& ) ), this, SLOT( NewChatSessionRequest(Communication::ChatSessionInterface&) ));
	    connect(im_connection_, SIGNAL( FriendRequestReceived(Communication::FriendRequestInterface&) ), this, SLOT( NewFriendRequest(Communication::FriendRequestInterface&) ));
        connect(im_connection_, SIGNAL( NewContact(const Communication::ContactInterface&) ), this, SLOT( OnNewContact(const Communication::ContactInterface&) ));
        connect(im_connection_, SIGNAL( ContactRemoved(const Communication::ContactInterface&) ), this, SLOT( OnContactRemoved(const Communication::ContactInterface&) ));
        */	
    }

    void LoginHelper::ConnectionFailed(Communication::ConnectionInterface &connection_interface)
    {

    }
    
}