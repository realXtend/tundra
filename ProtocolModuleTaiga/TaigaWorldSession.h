// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolUtilities_TaigaWorldSession_h
#define incl_ProtocolUtilities_TaigaWorldSession_h

#include "Login/LoginCredentials.h"

#include "ProtocolModuleTaigaApi.h"
#include "Interfaces/WorldSessionInterface.h"

#include <QUrl>

namespace TaigaProtocol
{
    class ProtocolModuleTaiga;

    class TAIGAPROTO_MODULE_API TaigaWorldSession : public ProtocolUtilities::WorldSessionInterface
    {
    public:
        //! Constuctor.
        explicit TaigaWorldSession(Foundation::Framework *framework);

        //! Destructor
        virtual ~TaigaWorldSession(void);

        //! WorldSessionInterface override
        bool StartSession(ProtocolUtilities::LoginCredentialsInterface *credentials, QUrl *serverEntryPointUrl);

        //! WorldSessionInterface override
        bool LoginToServer(const QString& address,
                           const QString& port,
                           const QString& identityUrl,
                           ProtocolUtilities::ConnectionThreadState *thread_state);

        //! WorldSessionInterface override
        QUrl ValidateUrl(const QString &urlString, const UrlType urlType);

        //! WorldSessionInterface override
        ProtocolUtilities::LoginCredentialsInterface* GetCredentials() const;

        //! WorldSessionInterface override
        QUrl GetServerEntryPointUrl() const;

        //! WorldSessionInterface override
        void GetWorldStream() const;

        //! WorldSessionInterface override
        void SetCredentials(ProtocolUtilities::LoginCredentialsInterface *newCredentials);

        //! WorldSessionInterface override
        void SetServerEntryPointUrl(const QUrl &newUrl);

    private:
        Q_DISABLE_COPY(TaigaWorldSession)

        ProtocolUtilities::TaigaCredentials *credentials_;
        QUrl serverEntryPointUrl_;

        //! Pointer to framework
        Foundation::Framework *framework_;

        //! Pointer to the opensim network interface.
        boost::weak_ptr<ProtocolModuleTaiga> networkTaiga_;
    };
}

#endif // incl_ProtocolUtilities_TaigaWorldSession_h
