// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoginHandler.h"
#include "RexLogicModule.h"
#include "WorldStream.h"
#include "ProtocolModuleOpenSim.h"
#include "OpenSimWorldSession.h"
#include "RealXtendWorldSession.h"
#include "ProtocolModuleTaiga.h"
#include "TaigaWorldSession.h"
#include "Interfaces/LoginCredentialsInterface.h"
#include "Login/LoginCredentials.h"
#include "Framework.h"
#include "ConfigurationManager.h"

#include <QStringList>
#include <QWebFrame>

#include "MemoryLeakCheck.h"

namespace RexLogic
{
    AbstractLoginHandler::AbstractLoginHandler() : credentials_(0), server_entry_point_url_(0)
    {
    }

    QUrl AbstractLoginHandler::ValidateServerUrl(QString &urlString)
    {
        ///\todo Move this functions elsewhere.
        QString sceme = urlString.mid(0,7);
        if (sceme != "http://")
        {
            urlString.insert(0, "http://");
            std::cout << "url scheme http:// was missing from url - Corrected" << std::endl;
        }
        QUrl returnUrl(urlString);
        if (returnUrl.isValid())
            return returnUrl;
        else
        {
            std::cout << "Invalid login url" << std::endl;
            return QUrl();
        }
    }

    OpenSimLoginHandler::OpenSimLoginHandler(RexLogicModule *owner) :
        owner_(owner), opensim_world_session_(0), realxtend_world_session_(0)
    {
    }

    OpenSimLoginHandler::~OpenSimLoginHandler()
    {
        delete credentials_;
        delete opensim_world_session_;
        delete realxtend_world_session_;
    }

    void OpenSimLoginHandler::SetLoginNotifier(QObject *notifier)
    {
        connect(notifier, SIGNAL(StartOsLogin(QMap<QString, QString>)), SLOT(ProcessOpenSimLogin(QMap<QString, QString>)));
        connect(notifier, SIGNAL(StartRexLogin(QMap<QString, QString>)), SLOT(ProcessRealXtendLogin(QMap<QString, QString>)));
        connect(notifier, SIGNAL(Disconnect()), SLOT(Logout()));
        connect(notifier, SIGNAL(Quit()), SLOT(Quit()));
    }

    void OpenSimLoginHandler::InstantiateWorldSession()
    {
        bool success = false;
        QString errorMessage = "";

        ProtocolUtilities::OpenSimCredentials *osCredentials = dynamic_cast<ProtocolUtilities::OpenSimCredentials *>(credentials_);
        if (osCredentials)
        {
            ProtocolUtilities::WorldStreamPtr stream = owner_->GetServerConnection();
            stream->UnregisterCurrentProtocolModule();
            stream->SetCurrentProtocolType(ProtocolUtilities::OpenSim);
            stream->SetConnectionType(ProtocolUtilities::DirectConnection);
            stream->StoreCredentials(osCredentials->GetIdentity().toStdString(), osCredentials->GetPassword().toStdString(), "");

            if (stream->PrepareCurrentProtocolModule() )
            {
                SAFE_DELETE(opensim_world_session_);
                assert(!opensim_world_session_);
                opensim_world_session_ = new OpenSimProtocol::OpenSimWorldSession(owner_->GetFramework());
                success = opensim_world_session_->StartSession(osCredentials, &server_entry_point_url_);
                if (success)
                {
                    // Save login credentials to config
                    Foundation::ConfigurationManagerPtr mgr = owner_->GetFramework()->GetConfigManager();
                    if (mgr->HasKey("Login", "server"))
                        mgr->SetSetting<std::string>("Login", "server", server_entry_point_url_.authority().toStdString());
                    else
                        mgr->DeclareSetting<std::string>("Login", "server", server_entry_point_url_.authority().toStdString());
                    if (mgr->HasKey("Login", "username"))
                        mgr->SetSetting<std::string>("Login", "username", osCredentials->GetIdentity().toStdString());
                    else
                        mgr->DeclareSetting<std::string>("Login", "username", osCredentials->GetIdentity().toStdString());
                }
                else
                    errorMessage = opensim_world_session_->GetConnectionThreadState()->errorMessage.c_str();
            }
        }
        else
        {
            // RealXtend login
            ProtocolUtilities::RealXtendCredentials *rexCredentials = dynamic_cast<ProtocolUtilities::RealXtendCredentials *>(credentials_);
            if (rexCredentials)
            {
                WorldStreamPtr stream = owner_->GetServerConnection();
                stream->UnregisterCurrentProtocolModule();
                stream->SetCurrentProtocolType(ProtocolUtilities::OpenSim);
                stream->SetConnectionType(ProtocolUtilities::AuthenticationConnection);
                stream->StoreCredentials(rexCredentials->GetIdentity().toStdString(),
                rexCredentials->GetPassword().toStdString(), rexCredentials->GetAuthenticationUrl().toString().toStdString());

                if (stream->PrepareCurrentProtocolModule() )
                {
                    SAFE_DELETE(realxtend_world_session_);
                    assert(!realxtend_world_session_);
                    realxtend_world_session_ = new OpenSimProtocol::RealXtendWorldSession(owner_->GetFramework());
                    success = realxtend_world_session_->StartSession(rexCredentials, &server_entry_point_url_);
                    if (success)
                    { 
                        // Save login credentials to config
                        Foundation::ConfigurationManagerPtr mgr = owner_->GetFramework()->GetConfigManager();
                        if (mgr->HasKey("Login", "rex_server"))
                            mgr->SetSetting<std::string>("Login", "rex_server", server_entry_point_url_.authority().toStdString());
                        else
                            mgr->DeclareSetting<std::string>("Login", "rex_server", server_entry_point_url_.authority().toStdString());
                        if (mgr->HasKey("Login", "auth_server"))
                            mgr->SetSetting<std::string>("Login", "auth_server", rexCredentials->GetAuthenticationUrl().authority().toStdString());
                        else
                            mgr->DeclareSetting<std::string>("Login", "auth_server", rexCredentials->GetAuthenticationUrl().host().toStdString());
                        if (mgr->HasKey("Login", "auth_name"))
                            mgr->SetSetting<std::string>("Login", "auth_name", rexCredentials->GetIdentity().toStdString());
                        else
                            mgr->DeclareSetting<std::string>("Login", "auth_name", rexCredentials->GetIdentity().toStdString());
                    }
                    else
                        errorMessage = realxtend_world_session_->GetConnectionThreadState()->errorMessage.c_str();
                }
            }
        }
    }

    void OpenSimLoginHandler::ProcessOpenSimLogin(QMap<QString,QString> map)
    {
        SAFE_DELETE(credentials_);
        credentials_ = new ProtocolUtilities::OpenSimCredentials();
        ProtocolUtilities::OpenSimCredentials *osCredentials = dynamic_cast<ProtocolUtilities::OpenSimCredentials *>(credentials_);
        if (osCredentials)
        {
            QString username = map["Username"];
            QStringList firstAndLast = username.split(" ");
            if (firstAndLast.length() == 2)
            {
                osCredentials->SetFirstName(firstAndLast.at(0));
                osCredentials->SetLastName(firstAndLast.at(1));
                osCredentials->SetPassword(map["Password"]);

                QString startLocation = map["StartLocation"];
                if (!startLocation.isEmpty())
                    osCredentials->SetStartLocation(startLocation);

                server_entry_point_url_ = ValidateServerUrl(map["WorldAddress"]);
                if (server_entry_point_url_.isValid())
                {
                    Logout();
                    emit LoginStarted();
                    InstantiateWorldSession();
                }
            }
            else
            {
                owner_->LogInfo("Username was not in form firstname lastname, could not perform login");
            }
        }
    }

    void OpenSimLoginHandler::ProcessRealXtendLogin(QMap<QString,QString> map)
    {
        SAFE_DELETE(credentials_);
        credentials_ = new ProtocolUtilities::RealXtendCredentials();
        ProtocolUtilities::RealXtendCredentials *rexCredentials = dynamic_cast<ProtocolUtilities::RealXtendCredentials *>(credentials_);
        if (rexCredentials)
        {
            rexCredentials->SetIdentity(map["Username"]);
            rexCredentials->SetPassword(map["Password"]);
            rexCredentials->SetAuthenticationUrl(ValidateServerUrl(map["AuthenticationAddress"]));

            QString startLocation = map["StartLocation"];
            if (!startLocation.isEmpty())
                rexCredentials->SetStartLocation(startLocation);

            server_entry_point_url_ = ValidateServerUrl(map["WorldAddress"]);
            if (server_entry_point_url_.isValid())
            {
                Logout();
                emit LoginStarted();
                InstantiateWorldSession();
            }
        }
    }

    void OpenSimLoginHandler::Logout()
    {
        owner_->LogoutAndDeleteWorld();
    }

    void OpenSimLoginHandler::Quit()
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->LogoutAndDeleteWorld();

        owner_->GetFramework()->Exit();
    }

    TaigaLoginHandler::TaigaLoginHandler(RexLogicModule *owner) :
        owner_(owner), taiga_world_session_(0)
    {
        credentials_ = new ProtocolUtilities::TaigaCredentials();
    }

    TaigaLoginHandler::~TaigaLoginHandler()
    {
        delete credentials_;
        delete taiga_world_session_;
    }

    void TaigaLoginHandler::SetLoginNotifier(QObject *notifier)
    {
        connect(notifier, SIGNAL(StartTaigaLogin(QWebFrame *)), SLOT( ProcessWebLogin(QWebFrame *)));
        connect(notifier, SIGNAL(StartTaigaLogin(QString)), SLOT(ProcessWebLogin(QString)));
    }

    void TaigaLoginHandler::InstantiateWorldSession()
    {
        bool success = false;
        QString errorMessage = "";

        ProtocolUtilities::WorldStreamPtr stream = owner_->GetServerConnection();
        stream->UnregisterCurrentProtocolModule();
        stream->SetCurrentProtocolType(ProtocolUtilities::Taiga);
        stream->SetConnectionType(ProtocolUtilities::DirectConnection);
        ProtocolUtilities::TaigaCredentials *tgCredentials = dynamic_cast<ProtocolUtilities::TaigaCredentials *>(credentials_);
        if (tgCredentials)
            stream->StoreCredentials(tgCredentials->GetIdentity().toStdString(), "", "");

        if (stream->PrepareCurrentProtocolModule())
        {
            SAFE_DELETE(taiga_world_session_);
            taiga_world_session_ = new TaigaProtocol::TaigaWorldSession(owner_->GetFramework());
            success = taiga_world_session_->StartSession(credentials_, &server_entry_point_url_);
            if (!success)
                errorMessage = taiga_world_session_->GetConnectionThreadState()->errorMessage.c_str();
        }
    }

    void TaigaLoginHandler::ProcessCommandParameterLogin(QString &entry_point_url)
    {
        server_entry_point_url_ = ValidateServerUrl(entry_point_url);
        dynamic_cast<ProtocolUtilities::TaigaCredentials *>(credentials_)->SetIdentityUrl("");
        if (server_entry_point_url_.isValid())
        {
            emit LoginStarted();
            InstantiateWorldSession();
        }
    }

    void TaigaLoginHandler::ProcessWebLogin(QWebFrame *web_frame)
    {
        int pos1, pos2;
        QString entry_point_url, identityUrl;
        QString returnValue = web_frame->evaluateJavaScript("ReturnSuccessValue()").toString();

        pos1 = returnValue.indexOf(QString("http://"), 0);
        pos2 = returnValue.indexOf(QString("?"), 0);
        entry_point_url = returnValue.mid(pos1, pos2-pos1);

        pos1 = returnValue.lastIndexOf(QString("&"));
        identityUrl = returnValue.mid(pos1+1, returnValue.length()-1);
        
        dynamic_cast<ProtocolUtilities::TaigaCredentials *>(credentials_)->SetIdentityUrl(identityUrl);
        server_entry_point_url_ = ValidateServerUrl(entry_point_url);
        if (server_entry_point_url_.isValid())
        {
            Logout();
            emit LoginStarted();
            InstantiateWorldSession();
        }
    }

    void TaigaLoginHandler::ProcessWebLogin(QString url)
    {
        server_entry_point_url_ = ValidateServerUrl(url);
        dynamic_cast<ProtocolUtilities::TaigaCredentials *>(credentials_)->SetIdentityUrl("");
        if (server_entry_point_url_.isValid())
        {
            emit LoginStarted();
            InstantiateWorldSession();
        }
    }

    void TaigaLoginHandler::Logout()
    {
        owner_->LogoutAndDeleteWorld();
    }

    void TaigaLoginHandler::Quit()
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->LogoutAndDeleteWorld();

        owner_->GetFramework()->Exit();
    }
}
