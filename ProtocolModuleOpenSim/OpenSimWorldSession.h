// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_OpenSimWorldSession_h
#define incl_Protocol_OpenSimWorldSession_h

#include "Login/LoginCredentials.h"

#include "ProtocolModuleOpenSimApi.h"
#include "Interfaces/WorldSessionInterface.h"

#include <QUrl>

namespace OpenSimProtocol
{
    class OSPROTO_MODULE_API OpenSimWorldSession : public ProtocolUtilities::WorldSessionInterface
    {
    public:
        //! RealXtendWorldSession constructor
        OpenSimWorldSession(Foundation::Framework *framework);

        //! RealXtendWorldSession deconstructor
        virtual ~OpenSimWorldSession(void);

        /* INHERITED FUNCTIONS FROM WorldSessionInterface */

        //! Login function
        bool StartSession(ProtocolUtilities::LoginCredentialsInterface *credentials, QUrl *serverEntryPointUrl);
        
        /**
         * Logs in to a reX server without the authentication procedure.
         * 
         * @param first_name is first part of given username. 
         * @param last_name is second part of given username.
         * @param address is world server ip-address (or dns-name?) does not contain port number.
         * @param port is a world server port (where connection is done). 
         * @return true if login was successfull false if not. 
         *
         * */
        bool LoginToServer(
            const QString& first_name,
            const QString& last_name,
            const QString& password,
            const QString& address,
            const QString& port,
			const QString& start_location,
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
        Q_DISABLE_COPY(OpenSimWorldSession)

        ProtocolUtilities::OpenSimCredentials *credentials_;
        QUrl serverEntryPointUrl_;

        //! Pointer to framework
        Foundation::Framework *framework_;

        //! Pointer to the opensim network interface.
        boost::weak_ptr<OpenSimProtocol::ProtocolModuleOpenSim> networkOpensim_;

        //! State of the connection procedure thread.
        ProtocolUtilities::ConnectionThreadState threadState_;
    };
}

#endif // incl_Protocol_OpenSimWorldSession_h