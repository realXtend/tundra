// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_RealXtendWorldSession_h
#define incl_Protocol_RealXtendWorldSession_h

#include "ProtocolModuleOpenSimApi.h"
#include "Interfaces/WorldSessionInterface.h"
#include "LoginCredentials.h"

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class ProtocolModuleOpenSim;

    class OSPROTO_MODULE_API RealXtendWorldSession : public ProtocolUtilities::WorldSessionInterface
    {
        Q_OBJECT

    public:
        //! Constructor
        RealXtendWorldSession(Foundation::Framework *framework);

        //! Destructor.
        virtual ~RealXtendWorldSession(void);

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
            const QString& start_location,
            ProtocolUtilities::ConnectionThreadState *thread_state);

        //! WorldSessionInterface override
        bool StartSession(const LoginCredentials &credentials, const QUrl &serverEntryPointUrl);

        //! WorldSessionInterface override
        QUrl ValidateUrl(const QString &urlString, const UrlType urlType);

        //! WorldSessionInterface override
        LoginCredentials GetCredentials() const;

        //! WorldSessionInterface override
        QUrl GetServerEntryPointUrl() const;

        //! WorldSessionInterface override
        void GetWorldStream() const;

        //! WorldSessionInterface override
        void SetCredentials(const LoginCredentials &credentials);

        //! WorldSessionInterface override
        void SetServerEntryPointUrl(const QUrl &newUrl);

    private:
        Q_DISABLE_COPY(RealXtendWorldSession)

        LoginCredentials credentials_;

        QUrl serverEntryPointUrl_;

        //! Pointer to framework
        Foundation::Framework *framework_;

        //! Pointer to the opensim network interface.
        boost::weak_ptr<ProtocolModuleOpenSim> networkOpensim_;

    private slots:
        void HandleLoginStateChange(int state);
    };
}

#endif // incl_Protocol_RealXtendWorldSession_h