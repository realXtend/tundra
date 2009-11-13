// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "ProtocolModuleOpenSim.h"
#include "RealXtendWorldSession.h"

namespace OpenSimProtocol
{

	RealXtendWorldSession::RealXtendWorldSession(Foundation::Framework *framework)
		: framework_(framework), credentials_(0), serverEntryPointUrl_(0)
	{
		networkOpensim_ = framework_->GetModuleManager()->GetModule<OpenSimProtocol::ProtocolModuleOpenSim>(Foundation::Module::MT_OpenSimProtocol);
	}

	RealXtendWorldSession::~RealXtendWorldSession()
	{

	}

	bool RealXtendWorldSession::StartSession(ProtocolUtilities::LoginCredentialsInterface *credentials, QUrl *serverEntryPointUrl)
	{
		bool success = false;
		RexLogic::RealXtendCredentials *testCredentials = dynamic_cast<RexLogic::RealXtendCredentials *>(credentials);
		if (testCredentials)
		{
			// Set Url and Credentials
			serverEntryPointUrl_ = ValidateUrl(serverEntryPointUrl->toString(), WorldSessionInterface::OpenSimServer);
			serverEntryPointUrl = &serverEntryPointUrl_;
			credentials_ = testCredentials;
			credentials_->SetAuthenticationUrl( ValidateUrl(credentials_->GetAuthenticationUrl().toString(), WorldSessionInterface::RealXtendAuthenticationServer) );

			// Try do RealXtend auth based login with ProtocolModuleOpenSim
			success = LoginToServer(std::string("realXtend"),
									std::string("realXtend"),
									credentials_->GetPassword().toStdString(),
									serverEntryPointUrl_.host().toStdString(),
									boost::lexical_cast<std::string>( serverEntryPointUrl_.port() ),
									credentials_->GetAuthenticationUrl().host().toStdString(),
									boost::lexical_cast<std::string>( credentials_->GetAuthenticationUrl().port() ),
									credentials_->GetIdentity().toStdString(),
									&threadState_);
		}
		else
		{
			ProtocolModuleOpenSim::LogInfo("Invalid credential type, must be RealXtendCredentials for RealXtendWorldSession");
			success = false;
		}

		return success;
	}

	bool RealXtendWorldSession::LoginToServer( const std::string& first_name,
											   const std::string& last_name,
											   const std::string& password,
											   const std::string& address,
											   const std::string& port,
											   const std::string& auth_server_address_noport,
											   const std::string& auth_server_port,
											   const std::string& auth_login,
											   ProtocolUtilities::ConnectionThreadState *thread_state )
	{
		// Get ProtocolModuleOpenSim
		boost::shared_ptr<OpenSimProtocol::ProtocolModuleOpenSim> spOpenSim = networkOpensim_.lock();

		if (spOpenSim.get())
		{

			std::string callMethod = "ClientAuthentication";
			spOpenSim->GetLoginWorker()->SetupXMLRPCLogin(first_name, last_name, password, address, port, callMethod, 
														  thread_state, auth_login, auth_server_address_noport, auth_server_port, true);
			spOpenSim->SetAuthenticationType(ProtocolUtilities::AT_RealXtend);
			// Start the login thread.
			boost::thread(boost::ref( *spOpenSim->GetLoginWorker() ));
		}
		else
		{
			LogInfo("Could not lock ProtocolModuleOpenSim");
			return false;
		}

		return true;
	}

	QUrl RealXtendWorldSession::ValidateUrl(const QString urlString, const UrlType urlType)
	{
		QUrl returnUrl(urlString);
		switch (urlType)
		{
			case WorldSessionInterface::OpenSimServer:
				if (returnUrl.port() == -1)
				{
					returnUrl.setPort(9000);
					ProtocolModuleOpenSim::LogInfo("OpenSimServer url had no port, using defalt 9000");
				}
				break;

			case WorldSessionInterface::OpenSimGridServer:
				if (returnUrl.port() == -1)
				{
					returnUrl.setPort(8002);
					ProtocolModuleOpenSim::LogInfo("OpenSimGridServer url had no port, using defalt 8002");
				}
				break;

			case WorldSessionInterface::RealXtendAuthenticationServer:
				if (returnUrl.port() == -1)
				{
					returnUrl.setPort(10001);
					ProtocolModuleOpenSim::LogInfo("RealXtendAuthenticationServer url had no port, using defalt 10001");
				}
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
		RexLogic::RealXtendCredentials *testCredentials = dynamic_cast<RexLogic::RealXtendCredentials *>(newCredentials);
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