// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolUtilities_TaigaWorldSession_h
#define incl_ProtocolUtilities_TaigaWorldSession_h

#include "Login/LoginCredentials.h"

#include "ProtocolModuleTaigaApi.h"
#include "Interfaces/WorldSessionInterface.h"

#include <QUrl>

namespace TaigaProtocol
{
	class TAIGAPROTO_MODULE_API TaigaWorldSession : public ProtocolUtilities::WorldSessionInterface
	{

	public:
		//! RealXtendWorldSession constructor
		TaigaWorldSession(Foundation::Framework *framework);

		//! RealXtendWorldSession deconstructor
		virtual ~TaigaWorldSession(void);

		/* INHERITED FUNCTIONS FROM WorldSessionInterface */

		//! Login function
		bool StartSession(ProtocolUtilities::LoginCredentialsInterface *credentials, QUrl *serverEntryPointUrl);

        //! Cable Beach style login, setting authentication done unlike normal LoginToServer
        bool LoginToServer(const QString& address,
						   const QString& port,
                           const QString& identityUrl,
						   ProtocolUtilities::ConnectionThreadState *thread_state);

		//! Make Url validation according to type
		QUrl ValidateUrl(const QString urlString, const UrlType urlType);

		//! Get login credentials
		ProtocolUtilities::LoginCredentialsInterface* GetCredentials() const;

		//! Get server entry point url. Used for xmlrpc login_to_simulator and authentication internally.
		QUrl GetServerEntryPointUrl() const;
		
		//! Get created WorldStream: void -> WorldStreamInterface when implemented
		void GetWorldStream() const;

		//! Set login credentials
		void SetCredentials(ProtocolUtilities::LoginCredentialsInterface *newCredentials);

		//! Set server entry point url
		void SetServerEntryPointUrl(const QUrl &newUrl);

	private:
		ProtocolUtilities::TaigaCredentials *credentials_;
		QUrl serverEntryPointUrl_;

		//! Pointer to framework
		Foundation::Framework *framework_;

		//! Pointer to the opensim network interface.
		boost::weak_ptr<TaigaProtocol::ProtocolModuleTaiga> networkTaiga_;
	};
}

#endif // incl_ProtocolUtilities_TaigaWorldSession_h