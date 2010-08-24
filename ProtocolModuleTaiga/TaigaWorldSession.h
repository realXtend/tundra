// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolUtilities_TaigaWorldSession_h
#define incl_ProtocolUtilities_TaigaWorldSession_h

#include "ProtocolModuleTaigaApi.h"
#include "Interfaces/WorldSessionInterface.h"
#include "LoginCredentials.h"

namespace Foundation
{
    class Framework;
}

namespace TaigaProtocol
{
    class ProtocolModuleTaiga;

    class TAIGAPROTO_MODULE_API TaigaWorldSession : public ProtocolUtilities::WorldSessionInterface
    {
        Q_OBJECT

    public:
        //! Constuctor.
        explicit TaigaWorldSession(Foundation::Framework *framework);

        //! Destructor
        virtual ~TaigaWorldSession(void);

        /**
         * Logs in to a Taiga server.
         * 
         * @param address Server address.
         * @param port Server pot.
         * @param identityUrl Taiga identity URL.
         * @param thread_state Login thread state.
         * @return true if login was successfull false if not.
         */
        bool LoginToServer(
            const QString& address,
            const QString& port,
            const QString& identityUrl,
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
        Q_DISABLE_COPY(TaigaWorldSession)

        LoginCredentials credentials_;
        QUrl serverEntryPointUrl_;

        //! Pointer to framework
        Foundation::Framework *framework_;

        //! Pointer to the opensim network interface.
        boost::weak_ptr<ProtocolModuleTaiga> networkTaiga_;
    };
}

#endif // incl_ProtocolUtilities_TaigaWorldSession_h
