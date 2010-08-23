// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "ProtocolModuleOpenSim.h"
#include "OpenSimWorldSession.h"
#include "Framework.h"
#include "ModuleManager.h"

namespace OpenSimProtocol
{
    OpenSimWorldSession::OpenSimWorldSession(Foundation::Framework *framework) :
        framework_(framework), credentials_(ProtocolUtilities::AT_OpenSim)
    {
        networkOpensim_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::ProtocolModuleOpenSim>();
    }

    OpenSimWorldSession::~OpenSimWorldSession()
    {
    }

    bool OpenSimWorldSession::StartSession(const LoginCredentials &credentials, const QUrl &serverEntryPointUrl)
    {
        bool success = false;
        if (credentials.GetType() == ProtocolUtilities::AT_OpenSim)
        {
            // Set Url and Credentials
            serverEntryPointUrl_ = ValidateUrl(serverEntryPointUrl.toString(), WorldSessionInterface::OpenSimServer);
            credentials_ = credentials;

            // Try do OpenSim login with ProtocolModuleOpenSim
            success = LoginToServer(
                credentials_.GetFirstName(),
                credentials_.GetLastName(),
                credentials_.GetPassword(),
                serverEntryPointUrl_.toString(),
                QString::number(serverEntryPointUrl_.port()),
                credentials_.GetStartLocation(),
                GetConnectionThreadState());
        }
        else
        {
            ProtocolModuleOpenSim::LogInfo("Invalid credential type, must be OpenSimCredentials for OpenSimWorldSession");
            success = false;
        }

        return success;
    }

    bool OpenSimWorldSession::LoginToServer(
        const QString& first_name,
        const QString& last_name,
        const QString& password,
        const QString& address,
        const QString& port,
        const QString& start_location,
        ProtocolUtilities::ConnectionThreadState *thread_state )
    {
        // Get ProtocolModuleOpenSim
        boost::shared_ptr<OpenSimProtocol::ProtocolModuleOpenSim> spOpenSim = networkOpensim_.lock();

        if (spOpenSim.get())
        {
            spOpenSim->GetLoginWorker()->PrepareOpenSimLogin(first_name, last_name, password, address, port, start_location, thread_state);
            spOpenSim->SetAuthenticationType(ProtocolUtilities::AT_OpenSim);
            // Start the thread.
            boost::thread(boost::ref( *spOpenSim->GetLoginWorker() ));
        }
        else
        {
            ProtocolModuleOpenSim::LogInfo("Could not lock ProtocolModuleOpenSim");
            return false;
        }

        return true;
    }


    QUrl OpenSimWorldSession::ValidateUrl(const QString &urlString, const UrlType urlType)
    {
        QUrl returnUrl(urlString);
        switch (urlType)
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

    LoginCredentials OpenSimWorldSession::GetCredentials() const
    {
        return credentials_;
    }

    QUrl OpenSimWorldSession::GetServerEntryPointUrl() const
    {
        return serverEntryPointUrl_;
    }

    void OpenSimWorldSession::GetWorldStream() const
    {
    }

    void OpenSimWorldSession::SetCredentials(const LoginCredentials &credentials)
    {
        if (credentials.GetType() == ProtocolUtilities::AT_OpenSim)
            credentials_ = credentials;
        else
            ProtocolModuleOpenSim::LogInfo("Could not set credentials, invalid type. Must be OpenSim for OpenSimWorldSession");
    }

    void OpenSimWorldSession::SetServerEntryPointUrl(const QUrl &newUrl)
    {
        serverEntryPointUrl_ = newUrl;
    }
}
