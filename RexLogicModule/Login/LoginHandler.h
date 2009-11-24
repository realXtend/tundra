// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_LoginHandler_h
#define incl_RexLogic_LoginHandler_h

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "AssetModule.h"

#include <QObject>
#include <QWebFrame>

#include "ProtocolModuleOpenSim.h"
#include "OpenSimWorldSession.h"
#include "RealXtendWorldSession.h"

#include "ProtocolModuleTaiga.h"
#include "TaigaWorldSession.h"
#include "Interfaces/LoginCredentialsInterface.h"

#include "Login/LoginCredentials.h"

namespace RexLogic
{
    class Login;
}

namespace RexLogic
{
	class AbstractLoginHandler : public QObject
	{
	
	Q_OBJECT

	public:
		AbstractLoginHandler(Foundation::Framework *framework, RexLogicModule *rexLogic);
		virtual void InstantiateWorldSession() = 0;
		virtual QUrl ValidateServerUrl(QString urlString);

		ProtocolUtilities::LoginCredentialsInterface *credentials_;
		QUrl serverEntryPointUrl_;

		/// Pointer to Framework
		Foundation::Framework *framework_;
        /// Pointerto RexLogicModule
		RexLogicModule *rexLogicModule_;

	signals:
        void LoginStarted();
		void LoginDone(bool success, QString &errorMessage);

	};

	class NaaliUI;
	class OpenSimLoginHandler : public AbstractLoginHandler
	{

	Q_OBJECT

	public:
		OpenSimLoginHandler(Foundation::Framework *framework, RexLogicModule *rexLogic);
		virtual ~OpenSimLoginHandler(void);
		void InstantiateWorldSession();

	public slots:
		void ProcessOpenSimLogin(QMap<QString,QString> map);
		void ProcessRealXtendLogin(QMap<QString,QString> map);

	private:
		//! Pointer to the opensim network interface.
        boost::weak_ptr<OpenSimProtocol::ProtocolModuleOpenSim> networkOpensim_;
		OpenSimProtocol::OpenSimWorldSession *openSimWorldSession_;
		OpenSimProtocol::RealXtendWorldSession *realXtendWorldSession_;

	};

	class TaigaLoginHandler : public AbstractLoginHandler
	{

	Q_OBJECT

	public:
		TaigaLoginHandler(Foundation::Framework *framework, RexLogicModule *rexLogic);
		virtual ~TaigaLoginHandler(void);
		void InstantiateWorldSession();

	public slots:
		void ProcessCommandParameterLogin(QString entryPointUrl);
		void ProcessWebLogin(QWebFrame *webFrame);

	private:
		boost::weak_ptr<TaigaProtocol::ProtocolModuleTaiga> networkTaiga_;
		TaigaProtocol::TaigaWorldSession *taigaWorldSession_;
	};

}

#endif //incl_RexLogic_LoginHandler_h