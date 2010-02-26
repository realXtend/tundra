// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_RealXtendWorldSession_h
#define incl_Protocol_RealXtendWorldSession_h

#include "Login/LoginCredentials.h"

#include "ProtocolModuleOpenSimApi.h"
#include "Interfaces/WorldSessionInterface.h"

#include <QUrl>

namespace OpenSimProtocol
{
    class OSPROTO_MODULE_API RealXtendWorldSession : public ProtocolUtilities::WorldSessionInterface
    {
    public:
        //! RealXtendWorldSession constructor
        RealXtendWorldSession(Foundation::Framework *framework);

        //! RealXtendWorldSession deconstructor
        virtual ~RealXtendWorldSession(void);

        /* INHERITED FUNCTIONS FROM WorldSessionInterface */

        //! Login function
        bool StartSession(ProtocolUtilities::LoginCredentialsInterface *credentials, QUrl *serverEntryPointUrl);

        /**
         * Logs in to a reX server using the authentication procedure.
         * 
         * @param first_name is first part of given username. 
         * @param last_name is second part of given username.
         * @param address is world server ip-address (or dns-name?) does not contain port number.
         * @param port is a world server port (where connection is done). 
         * @param auth_server_address_noport is authentication server ip-address 
         * @param auth_server_port
         * @param auth_login is a login name which will be used to login authentication server. 
         * @return true if login was successfull false if not. 
         *
         * */
        bool LoginToServer(
            const QString& password,
            const QString& address,
            const QString& port,
            const QString& auth_server_address_noport,
            const QString& auth_server_port,
            const QString& auth_login,
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
        Q_DISABLE_COPY(RealXtendWorldSession)

        ProtocolUtilities::RealXtendCredentials *credentials_;
        QUrl serverEntryPointUrl_;

        //! Pointer to framework
        Foundation::Framework *framework_;

        //! Pointer to the opensim network interface.
        boost::weak_ptr<OpenSimProtocol::ProtocolModuleOpenSim> networkOpensim_;

    };
}

#endif // incl_Protocol_RealXtendWorldSession_h