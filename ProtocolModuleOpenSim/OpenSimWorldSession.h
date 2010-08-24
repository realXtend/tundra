// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_OpenSimWorldSession_h
#define incl_Protocol_OpenSimWorldSession_h

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

    class OSPROTO_MODULE_API OpenSimWorldSession : public ProtocolUtilities::WorldSessionInterface
    {
        Q_OBJECT

    public:
        /** Constructor.
         *  @param framework Framework.
         */
        explicit OpenSimWorldSession(Foundation::Framework *framework);

        //! Destructor.
        virtual ~OpenSimWorldSession();

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
        Q_DISABLE_COPY(OpenSimWorldSession)

        LoginCredentials credentials_;

        QUrl serverEntryPointUrl_;

        //! Pointer to framework
        Foundation::Framework *framework_;

        //! Pointer to the opensim network interface.
        boost::weak_ptr<ProtocolModuleOpenSim> networkOpensim_;

        //! State of the connection procedure thread.
        ProtocolUtilities::ConnectionThreadState threadState_;

    private slots:
        void HandleLoginStateChange(int state);
    };
}

#endif // incl_Protocol_OpenSimWorldSession_h
