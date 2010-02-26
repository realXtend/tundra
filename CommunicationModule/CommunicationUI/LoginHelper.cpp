// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoginHelper.h"

#include "Credentials.h"
#include "CommunicationService.h"
#include "ConnectionInterface.h"
#include "CoreDefines.h"
#include "CoreException.h"

#include <QUrl>

#include "MemoryLeakCheck.h"

namespace UiHelpers
{
    LoginHelper::LoginHelper()
        : QObject(),
          login_ui_(0),
          communication_service_(Communication::CommunicationService::GetInstance()),
          im_connection_(0),
          error_message_(""),
          username_(""),
          server_(""),
          password_("")
    {

    }

    LoginHelper::~LoginHelper()
    {
        login_ui_ = 0;
        im_connection_ = 0;
        communication_service_ = 0;
    }

    void LoginHelper::SetupUi(Ui::LoginWidget *login_ui)
    { 
        SAFE_DELETE(login_ui_); 
        login_ui_ = login_ui;
    }

    QString LoginHelper::GetErrorMessage()
    { 
        return error_message_;
    }

    QMap<QString, QString> LoginHelper::GetPreviousCredentials()
    {
        QMap<QString, QString> data_map;
        data_map["username"] = username_;
        data_map["server"] = server_;
        data_map["password"] = password_;
        return data_map;
    }

    void LoginHelper::TryLogin()
    {
        assert(login_ui_);

        username_ = login_ui_->usernameLineEdit->text();
        server_ = login_ui_->serverLineEdit->text();
        password_ = login_ui_->passwordLineEdit->text();

        // Username validation
        if (!username_.contains("@"))
            username_.append(QString("@%1").arg(server_));

        // Server and port validation
        if (!server_.startsWith("http://") || !server_.startsWith("https://"))
            server_ = "http://" + server_;
        QUrl server_url(server_);
        if (server_url.port() == -1)
            server_url.setPort(5222);

        // Create the credentials
        Communication::Credentials credentials;
        credentials.SetProtocol("jabber");
        credentials.SetUserID(username_);
        credentials.SetPassword(password_);
        credentials.SetServer(server_url.host());
        credentials.SetPort(server_url.port());

        server_ = server_url.authority();

        try
        {
            im_connection_ = 0; // CommunicationService will delete this
            im_connection_ = communication_service_->OpenConnection(credentials);
        }
        catch (Exception &/*e*/) { /* e.what() for error */ }

        connect(im_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( ConnectionEstablished(Communication::ConnectionInterface&) ));
        connect(im_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( ConnectionFailed(Communication::ConnectionInterface&) ));
        emit StateChange(UiDefines::UiStates::Connecting);
    }

    void LoginHelper::LoginCanceled()
    {
        error_message_ = "Connecting operation canceled";
        im_connection_->Close();
        emit StateChange(UiDefines::UiStates::Disconnected);
    }

    void LoginHelper::ConnectionFailed(Communication::ConnectionInterface &connection_interface)
    {
        error_message_ = "Connecting failed, please check your credentials";
        im_connection_->Close();
        emit StateChange(UiDefines::UiStates::Disconnected);
    }

    void LoginHelper::ConnectionEstablished(Communication::ConnectionInterface &connection_interface)
    {
        error_message_ = "";
        emit StateChange(UiDefines::UiStates::Connected);
    }

}