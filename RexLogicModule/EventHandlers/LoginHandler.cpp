/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LoginHandler.cpp
 *  @brief  Performs login processes supported by Naali: OpenSim, RealXtend and Taiga weblogin.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoginHandler.h"
#include "RexLogicModule.h"

#include "WorldStream.h"
#include "OpenSimWorldSession.h"
#include "RealXtendWorldSession.h"
#include "TaigaWorldSession.h"
#include "Framework.h"
#include "ConfigurationManager.h"

#include <QStringList>
#include <QWebFrame>

#include "MemoryLeakCheck.h"

namespace
{
    /// Validates that the URL string has http shceme and generates QUrl from it.
    QUrl ValidateServerUrl(const QString &urlString)
    {
        QUrl url;
        QString shceme = urlString.mid(0,7);
        if (shceme != "http://")
        {
            RexLogic::RexLogicModule::LogInfo("url scheme http:// was missing from url - corrected");
            url = "http://" + urlString;
        }
        else
            url = urlString;

        if (!url.isValid())
        {
            RexLogic::RexLogicModule::LogError("Invalid login url.");
            return QUrl();
        }

        return url;
    }
}

namespace RexLogic
{
    LoginHandler::LoginHandler(RexLogicModule *owner) :
        owner_(owner), world_session_(0), credentials_(ProtocolUtilities::AT_OpenSim)
    {
    }

    LoginHandler::~LoginHandler()
    {
        SAFE_DELETE(world_session_);
    }

    void LoginHandler::ProcessLoginData(const QMap<QString, QString> &data)
    {
        QString type = data["AvatarType"];
        if (type == "OpenSim")
        {
            credentials_.SetType(ProtocolUtilities::AT_OpenSim);
            QString username = data["Username"];
            QStringList firstAndLast = username.split(" ");
            if (firstAndLast.length() == 2)
            {
                credentials_.SetFirstName(firstAndLast.at(0));
                credentials_.SetLastName(firstAndLast.at(1));
                credentials_.SetPassword(data["Password"]);

                QString startLocation = data["StartLocation"];
                if (!startLocation.isEmpty())
                    credentials_.SetStartLocation(startLocation);

                server_entry_point_url_ = ValidateServerUrl(data["WorldAddress"]);
                if (server_entry_point_url_.isValid())
                {
                    Logout();
                    emit LoginStarted();
                    StartWorldSession();
                }
            }
            else
            {
                RexLogicModule::LogError("Username was not in form firstname lastname, could not perform login");
            }
        }
        else if (type == "RealXtend")
        {
            credentials_.SetType(ProtocolUtilities::AT_RealXtend);
            credentials_.SetIdentity(data["Username"]);
            credentials_.SetPassword(data["Password"]);
            credentials_.SetAuthenticationUrl(ValidateServerUrl(data["AuthenticationAddress"]));

            QString startLocation = data["StartLocation"];
            if (!startLocation.isEmpty())
                credentials_.SetStartLocation(startLocation);

            server_entry_point_url_ = ValidateServerUrl(data["WorldAddress"]);
            if (server_entry_point_url_.isValid())
            {
                Logout();
                emit LoginStarted();
                StartWorldSession();
            }
        }
        else
        {
            RexLogicModule::LogError("Could not find avatar type in login info map.");
            //emit LoginFailed("msg");
        }
    }

    void LoginHandler::ProcessLoginData(QWebFrame *frame)
    {
        int pos1, pos2;
        QString entry_point_url, identityUrl;
        QString returnValue = frame->evaluateJavaScript("ReturnSuccessValue()").toString();

        pos1 = returnValue.indexOf("http://", 0);
        pos2 = returnValue.indexOf('?', 0);
        entry_point_url = returnValue.mid(pos1, pos2-pos1);

        pos1 = returnValue.lastIndexOf('&');
        identityUrl = returnValue.mid(pos1+1, returnValue.length()-1);

        credentials_.SetIdentity(identityUrl);
        server_entry_point_url_ = ValidateServerUrl(entry_point_url);
        if (server_entry_point_url_.isValid())
        {
            credentials_.SetType(ProtocolUtilities::AT_Taiga);
            Logout();
            emit LoginStarted();
            StartWorldSession();
        }
    }

    void LoginHandler::ProcessLoginData(const QString &url)
    {
        server_entry_point_url_ = ValidateServerUrl(url);
        credentials_.SetIdentity("");
        if (server_entry_point_url_.isValid())
        {
            credentials_.SetType(ProtocolUtilities::AT_Taiga);
            emit LoginStarted();
            StartWorldSession();
        }
    }

    void LoginHandler::StartWorldSession()
    {
        bool success = false;

        if (credentials_.GetType() == ProtocolUtilities::AT_OpenSim)
        {
            ProtocolUtilities::WorldStreamPtr stream = owner_->GetServerConnection();
            stream->UnregisterCurrentProtocolModule();
            stream->SetCurrentProtocolType(ProtocolUtilities::OpenSim);
            stream->SetConnectionType(ProtocolUtilities::DirectConnection);
            stream->StoreCredentials(credentials_.GetIdentity().toStdString(), credentials_.GetPassword().toStdString(), "");

            if (stream->PrepareCurrentProtocolModule() )
            {
                SAFE_DELETE(world_session_);
                world_session_ = new OpenSimProtocol::OpenSimWorldSession(owner_->GetFramework());

                success = world_session_->StartSession(credentials_, server_entry_point_url_);
                if (success)
                {
                    // Save login credentials to config
                    Foundation::ConfigurationManagerPtr mgr = owner_->GetFramework()->GetConfigManager();
                    if (mgr->HasKey("Login", "server"))
                        mgr->SetSetting<std::string>("Login", "server", server_entry_point_url_.authority().toStdString());
                    else
                        mgr->DeclareSetting<std::string>("Login", "server", server_entry_point_url_.authority().toStdString());
                    if (mgr->HasKey("Login", "username"))
                        mgr->SetSetting<std::string>("Login", "username", credentials_.GetIdentity().toStdString());
                    else
                        mgr->DeclareSetting<std::string>("Login", "username", credentials_.GetIdentity().toStdString());
                }
                else
                {
                    QString errorMsg = world_session_->GetConnectionThreadState()->errorMessage.c_str();
                    emit LoginFailed(errorMsg);
                    RexLogicModule::LogError(errorMsg.toStdString());
                }
            }
        }
        else if (credentials_.GetType() == ProtocolUtilities::AT_RealXtend)
        {
            WorldStreamPtr stream = owner_->GetServerConnection();
            stream->UnregisterCurrentProtocolModule();
            stream->SetCurrentProtocolType(ProtocolUtilities::OpenSim);
            stream->SetConnectionType(ProtocolUtilities::AuthenticationConnection);
            stream->StoreCredentials(credentials_.GetIdentity().toStdString(),
            credentials_.GetPassword().toStdString(), credentials_.GetAuthenticationUrl().toString().toStdString());

            if (stream->PrepareCurrentProtocolModule())
            {
                SAFE_DELETE(world_session_);
                world_session_ = new OpenSimProtocol::RealXtendWorldSession(owner_->GetFramework());

                success = world_session_->StartSession(credentials_, server_entry_point_url_);
                if (success)
                {
                    // Save login credentials to config
                    Foundation::ConfigurationManagerPtr mgr = owner_->GetFramework()->GetConfigManager();
                    if (mgr->HasKey("Login", "rex_server"))
                        mgr->SetSetting<std::string>("Login", "rex_server", server_entry_point_url_.authority().toStdString());
                    else
                        mgr->DeclareSetting<std::string>("Login", "rex_server", server_entry_point_url_.authority().toStdString());
                    if (mgr->HasKey("Login", "auth_server"))
                        mgr->SetSetting<std::string>("Login", "auth_server", credentials_.GetAuthenticationUrl().authority().toStdString());
                    else
                        mgr->DeclareSetting<std::string>("Login", "auth_server", credentials_.GetAuthenticationUrl().host().toStdString());
                    if (mgr->HasKey("Login", "auth_name"))
                        mgr->SetSetting<std::string>("Login", "auth_name", credentials_.GetIdentity().toStdString());
                    else
                        mgr->DeclareSetting<std::string>("Login", "auth_name", credentials_.GetIdentity().toStdString());
                }
                else
                {
                    QString errorMsg = world_session_->GetConnectionThreadState()->errorMessage.c_str();
                    emit LoginFailed(errorMsg);
                    RexLogicModule::LogError(errorMsg.toStdString());
                }
            }
        }
        else if (credentials_.GetType() == ProtocolUtilities::AT_Taiga)
        {
            bool success = false;
            QString errorMessage = "";

            ProtocolUtilities::WorldStreamPtr stream = owner_->GetServerConnection();
            stream->UnregisterCurrentProtocolModule();
            stream->SetCurrentProtocolType(ProtocolUtilities::Taiga);
            stream->SetConnectionType(ProtocolUtilities::DirectConnection);
            stream->StoreCredentials(credentials_.GetIdentity().toStdString(), "", "");

            if (stream->PrepareCurrentProtocolModule())
            {
                SAFE_DELETE(world_session_);
                world_session_ = new TaigaProtocol::TaigaWorldSession(owner_->GetFramework());
                success = world_session_->StartSession(credentials_, server_entry_point_url_);
                if (!success)
                {
                    QString errorMsg = world_session_->GetConnectionThreadState()->errorMessage.c_str();
                    emit LoginFailed(errorMsg);
                    RexLogicModule::LogError(errorMsg.toStdString());
                }
            }
        }
    }

    void LoginHandler::Logout()
    {
        owner_->LogoutAndDeleteWorld();
    }

    void LoginHandler::Quit()
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->LogoutAndDeleteWorld();

        owner_->GetFramework()->Exit();
    }
}
