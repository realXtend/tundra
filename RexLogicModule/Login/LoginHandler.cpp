// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WorldStream.h"

#include "Login/LoginHandler.h"

namespace RexLogic
{
	AbstractLoginHandler::AbstractLoginHandler(Foundation::Framework *framework, RexLogicModule *rexLogic)
		: framework_(framework), rexLogicModule_(rexLogic), credentials_(0), serverEntryPointUrl_(0)
	{

	}

	QUrl AbstractLoginHandler::ValidateServerUrl(QString urlString)
	{
		QString sceme = urlString.midRef(0,7).toString();
		if (sceme != "http://")
		{
			urlString.insert(0, "http://");
			rexLogicModule_->LogInfo("http:// was missing from url, corrected");
		}
		QUrl returnUrl(urlString);
		if (returnUrl.isValid())
			return returnUrl;
		else
		{
			rexLogicModule_->LogInfo("invalid url");
			return QUrl();
		}
	}

	OpenSimLoginHandler::OpenSimLoginHandler(Foundation::Framework *framework, RexLogicModule *rexLogic)
		: AbstractLoginHandler(framework, rexLogic), openSimWorldSession_(0), realXtendWorldSession_(0)
	{

	}

	OpenSimLoginHandler::~OpenSimLoginHandler(void)
	{
		delete credentials_;
		if (realXtendWorldSession_)
			delete openSimWorldSession_;
		if (realXtendWorldSession_)
			delete realXtendWorldSession_;
	}

	void OpenSimLoginHandler::InstantiateWorldSession()
	{
		bool success = false;
        QString errorMessage = "";

		OpenSimCredentials *osCredentials = dynamic_cast<OpenSimCredentials *>(credentials_);
		if (osCredentials)
		{
			rexLogicModule_->GetServerConnection()->UnregisterCurrentProtocolModule();
			rexLogicModule_->GetServerConnection()->SetCurrentProtocolType(ProtocolUtilities::OpenSim);
			rexLogicModule_->GetServerConnection()->SetConnectionType(ProtocolUtilities::DirectConnection);
			rexLogicModule_->GetServerConnection()->StoreCredentials(osCredentials->GetIdentity().toStdString(),
			                                                         osCredentials->GetPassword().toStdString(),
			                                                         "");
			    
			if ( rexLogicModule_->GetServerConnection()->PrepareCurrentProtocolModule() )
			{	
				openSimWorldSession_ = new OpenSimProtocol::OpenSimWorldSession(framework_);
				success = openSimWorldSession_->StartSession(osCredentials, &serverEntryPointUrl_);
				if (success)
				{
					// Save login credentials to config
					if ( framework_->GetConfigManager()->HasKey(std::string("Login"), std::string("server")) )
						framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("server"), serverEntryPointUrl_.authority().toStdString());
					else
						framework_->GetConfigManager()->DeclareSetting<std::string>(std::string("Login"), std::string("server"), serverEntryPointUrl_.authority().toStdString());
					if ( framework_->GetConfigManager()->HasKey(std::string("Login"), std::string("username")) )
						framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("username"), osCredentials->GetIdentity().toStdString());
					else
						framework_->GetConfigManager()->DeclareSetting<std::string>(std::string("Login"), std::string("username"), osCredentials->GetIdentity().toStdString());
				}
                else
                    errorMessage = QString(openSimWorldSession_->GetConnectionThreadState()->errorMessage.c_str());
			}
		}
		else
		{
			// RealXtend login
			RealXtendCredentials *rexCredentials = dynamic_cast<RealXtendCredentials *>(credentials_);
			if (rexCredentials)
			{
				rexLogicModule_->GetServerConnection()->UnregisterCurrentProtocolModule();
				rexLogicModule_->GetServerConnection()->SetCurrentProtocolType(ProtocolUtilities::OpenSim);
				rexLogicModule_->GetServerConnection()->SetConnectionType(ProtocolUtilities::AuthenticationConnection);
			    rexLogicModule_->GetServerConnection()->StoreCredentials(rexCredentials->GetIdentity().toStdString(),
			                                                             rexCredentials->GetPassword().toStdString(),
			                                                             rexCredentials->GetAuthenticationUrl().toString().toStdString());
			    
				if ( rexLogicModule_->GetServerConnection()->PrepareCurrentProtocolModule() )
				{	
					realXtendWorldSession_ = new OpenSimProtocol::RealXtendWorldSession(framework_);
					success = realXtendWorldSession_->StartSession(rexCredentials, &serverEntryPointUrl_);
					if (success)
					{ 
						// Save login credentials to config
						if ( framework_->GetConfigManager()->HasKey(std::string("Login"), std::string("rex_server")) )
							framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("rex_server"), serverEntryPointUrl_.authority().toStdString());
						else
							framework_->GetConfigManager()->DeclareSetting<std::string>(std::string("Login"), std::string("rex_server"), serverEntryPointUrl_.authority().toStdString());
						if ( framework_->GetConfigManager()->HasKey(std::string("Login"), std::string("auth_server")) )
							framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("auth_server"), rexCredentials->GetAuthenticationUrl().authority().toStdString());
						else
							framework_->GetConfigManager()->DeclareSetting<std::string>(std::string("Login"), std::string("auth_server"), rexCredentials->GetAuthenticationUrl().host().toStdString());
						if ( framework_->GetConfigManager()->HasKey(std::string("Login"), std::string("auth_name")) )
							framework_->GetConfigManager()->SetSetting<std::string>(std::string("Login"), std::string("auth_name"), rexCredentials->GetIdentity().toStdString());
						else
							framework_->GetConfigManager()->DeclareSetting<std::string>(std::string("Login"), std::string("auth_name"), rexCredentials->GetIdentity().toStdString());
					}
                    else
                        errorMessage = QString(realXtendWorldSession_->GetConnectionThreadState()->errorMessage.c_str());
				}
			}
		}

		emit( LoginDone(success, errorMessage) );
	}

	void OpenSimLoginHandler::ProcessOpenSimLogin(QMap<QString,QString> map)
	{
		credentials_ = new OpenSimCredentials();
		OpenSimCredentials *osCredentials = dynamic_cast<OpenSimCredentials *>(credentials_);
		if (osCredentials)
		{
			QString username = map["Username"];
			QStringList firstAndLast = username.split(" ");
			if (firstAndLast.length() == 2)
			{
				osCredentials->SetFirstName(firstAndLast.at(0));
				osCredentials->SetLastName(firstAndLast.at(1));
				osCredentials->SetPassword(map["Password"]);
				serverEntryPointUrl_ = ValidateServerUrl(map["WorldAddress"]);
				if (serverEntryPointUrl_.isValid())
                {
                    emit( LoginStarted() );
					InstantiateWorldSession();
                }
			}
			else
			{
			    rexLogicModule_->LogInfo("Username was not in form firstname lastname, could not perform login");
		    }		
		}
	}

	void OpenSimLoginHandler::ProcessRealXtendLogin(QMap<QString,QString> map)
	{
		credentials_ = new RealXtendCredentials();
		RealXtendCredentials *rexCredentials = dynamic_cast<RealXtendCredentials *>(credentials_);
		if (rexCredentials)
		{
			rexCredentials->SetIdentity(map["Username"]);
			rexCredentials->SetPassword(map["Password"]);
			rexCredentials->SetAuthenticationUrl(ValidateServerUrl(map["AuthenticationAddress"]));
			serverEntryPointUrl_ = ValidateServerUrl(map["WorldAddress"]);
			if (serverEntryPointUrl_.isValid())
            {
                emit( LoginStarted() );
				InstantiateWorldSession();
            }
		}
	}


	TaigaLoginHandler::TaigaLoginHandler(Foundation::Framework *framework, RexLogicModule *rexLogic)
		: AbstractLoginHandler(framework, rexLogic), taigaWorldSession_(0)
	{
		credentials_ = new TaigaCredentials();
	}

	TaigaLoginHandler::~TaigaLoginHandler(void)
	{
		delete credentials_;
		if (taigaWorldSession_)
			delete taigaWorldSession_;
	}

	void TaigaLoginHandler::InstantiateWorldSession()
	{
		bool success = false;
        QString errorMessage = "";

		rexLogicModule_->GetServerConnection()->UnregisterCurrentProtocolModule();
		rexLogicModule_->GetServerConnection()->SetCurrentProtocolType(ProtocolUtilities::Taiga);
		rexLogicModule_->GetServerConnection()->SetConnectionType(ProtocolUtilities::DirectConnection);
		RexLogic::TaigaCredentials *tgCredentials = dynamic_cast<RexLogic::TaigaCredentials *>(credentials_);	
		if (tgCredentials)	    
    	{
    		rexLogicModule_->GetServerConnection()->StoreCredentials(
    		    tgCredentials->GetIdentity().toStdString(),
    		    "",
    		    "");
        }
        if ( rexLogicModule_->GetServerConnection()->PrepareCurrentProtocolModule() )
		{
			taigaWorldSession_ = new TaigaProtocol::TaigaWorldSession(framework_);
			success = taigaWorldSession_->StartSession(credentials_, &serverEntryPointUrl_);
            if (!success)
                errorMessage = QString(taigaWorldSession_->GetConnectionThreadState()->errorMessage.c_str());
		}

		emit( LoginDone(success, errorMessage) );
	}

	void TaigaLoginHandler::ProcessCommandParameterLogin(QString entryPointUrl)
	{
		serverEntryPointUrl_ = ValidateServerUrl(entryPointUrl);
		dynamic_cast<TaigaCredentials *>(credentials_)->SetIdentityUrl("NotNeeded");
		if (serverEntryPointUrl_.isValid())
		{
			emit( LoginStarted() );
			InstantiateWorldSession();
		}
	}

	void TaigaLoginHandler::ProcessWebLogin(QWebFrame *webFrame)
	{
		int pos1, pos2;
		QString entryPointUrl, identityUrl;
		QString returnValue = webFrame->evaluateJavaScript("ReturnSuccessValue()").toString();

		pos1 = returnValue.indexOf(QString("http://"), 0);
		pos2 = returnValue.indexOf(QString("?"), 0);
		entryPointUrl = returnValue.midRef(pos1, pos2-pos1).toString();

		pos1 = returnValue.lastIndexOf(QString("&"));
		identityUrl = returnValue.midRef(pos1+1, returnValue.length()-1).toString();
		
		dynamic_cast<TaigaCredentials *>(credentials_)->SetIdentityUrl(identityUrl);
		serverEntryPointUrl_ = ValidateServerUrl(entryPointUrl);
		if (serverEntryPointUrl_.isValid())
        {
            emit( LoginStarted() );
			InstantiateWorldSession();
        }
	}
}