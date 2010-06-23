// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "ProtocolModuleOpenSim.h"
#include "RealXtendWorldSession.h"
#include "Framework.h"
#include "ModuleManager.h"

namespace OpenSimProtocol
{
    RealXtendWorldSession::RealXtendWorldSession(Foundation::Framework *framework) :
        framework_(framework), credentials_(0), serverEntryPointUrl_(0)
    {
        networkOpensim_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::ProtocolModuleOpenSim>();
    }

    RealXtendWorldSession::~RealXtendWorldSession()
    {
    }

    bool RealXtendWorldSession::StartSession(ProtocolUtilities::LoginCredentialsInterface *credentials, QUrl *serverEntryPointUrl)
    {
        bool success = false;
        ProtocolUtilities::RealXtendCredentials *testCredentials = dynamic_cast<ProtocolUtilities::RealXtendCredentials *>(credentials);
        if (testCredentials)
        {
            // Set Url and Credentials
            serverEntryPointUrl_ = ValidateUrl(serverEntryPointUrl->toString(), WorldSessionInterface::OpenSimServer);
            serverEntryPointUrl = &serverEntryPointUrl_;
            credentials_ = testCredentials;
            credentials_->SetAuthenticationUrl( ValidateUrl(credentials_->GetAuthenticationUrl().toString(), WorldSessionInterface::RealXtendAuthenticationServer) );

            // Try do RealXtend auth based login with ProtocolModuleOpenSim
            success = LoginToServer(
                credentials_->GetPassword(),
                serverEntryPointUrl_.host(),
                QString::number(serverEntryPointUrl_.port()),
                credentials_->GetAuthenticationUrl().host(),
                QString::number(credentials_->GetAuthenticationUrl().port()),
                credentials_->GetIdentity(),
                credentials_->GetStartLocation(),
                GetConnectionThreadState());
        }
        else
        {
            ProtocolModuleOpenSim::LogInfo("Invalid credential type, must be RealXtendCredentials for RealXtendWorldSession");
            success = false;
        }

        return success;
    }

    bool RealXtendWorldSession::LoginToServer(
        const QString& password,
        const QString& address,
        const QString& port,
        const QString& auth_server_address_noport,
        const QString& auth_server_port,
        const QString& auth_login,
        const QString& start_location,
        ProtocolUtilities::ConnectionThreadState *thread_state )
    {
        // Get ProtocolModuleOpenSim
        boost::shared_ptr<OpenSimProtocol::ProtocolModuleOpenSim> spOpenSim = networkOpensim_.lock();

        if (spOpenSim.get())
        {
            spOpenSim->GetLoginWorker()->PrepareRealXtendLogin(password, address, port, thread_state, auth_login, 
                auth_server_address_noport, auth_server_port, start_location);
            spOpenSim->SetAuthenticationType(ProtocolUtilities::AT_RealXtend);
            // Start the login thread.
            boost::thread(boost::ref(*spOpenSim->GetLoginWorker()));
        }
        else
        {
            ProtocolModuleOpenSim::LogInfo("Could not lock ProtocolModuleOpenSim");
            return false;
        }

        return true;
    }

    QUrl RealXtendWorldSession::ValidateUrl(const QString &urlString, const UrlType urlType)
    {
        QUrl returnUrl(urlString);
        switch(urlType)
        {
        case WorldSessionInterface::OpenSimServer:
            if (returnUrl.port() == -1)
            {
                returnUrl.setPort(9000);
                ProtocolModuleOpenSim::LogInfo("OpenSimServer url had no port, using default 9000");
            }
            break;

        case WorldSessionInterface::OpenSimGridServer:
            if (returnUrl.port() == -1)
            {
                returnUrl.setPort(8002);
                ProtocolModuleOpenSim::LogInfo("OpenSimGridServer url had no port, using default 8002");
            }
            break;

        case WorldSessionInterface::RealXtendAuthenticationServer:
            if (returnUrl.port() == -1)
            {
                returnUrl.setPort(10001);
                ProtocolModuleOpenSim::LogInfo("RealXtendAuthenticationServer url had no port, using default 10001");
            }
            break;
        default:
            break;
        }

        if (returnUrl.isValid())
            return returnUrl;
        else
        {
            ProtocolModuleOpenSim::LogInfo("Invalid connection url");
            return QUrl();
        }
    }

    ProtocolUtilities::LoginCredentialsInterface* RealXtendWorldSession::GetCredentials() const
    {
        return credentials_;
    }

    QUrl RealXtendWorldSession::GetServerEntryPointUrl() const
    {
        return serverEntryPointUrl_;
    }

    void RealXtendWorldSession::GetWorldStream() const
    {
    }

    void RealXtendWorldSession::SetCredentials(ProtocolUtilities::LoginCredentialsInterface *newCredentials)
    {
        ProtocolUtilities::RealXtendCredentials *testCredentials = dynamic_cast<ProtocolUtilities::RealXtendCredentials *>(newCredentials);
        if (testCredentials)
            credentials_ = testCredentials;
        else
            ProtocolModuleOpenSim::LogInfo("Could not set credentials, invalid type. Must be RealXtendCredentials for RealXtendWorldSession");
    }

    void RealXtendWorldSession::SetServerEntryPointUrl(const QUrl &newUrl)
    {
        serverEntryPointUrl_ = newUrl;
    }
}
