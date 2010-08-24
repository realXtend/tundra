// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolUtilities_WorldSessionInterface_h
#define incl_ProtocolUtilities_WorldSessionInterface_h

#include "LoginCredentials.h"
#include "NetworkEvents.h"

#include <QObject>
#include <QUrl>

namespace ProtocolUtilities
{
    class WorldSessionInterface : public QObject
    {
        Q_OBJECT 

    public:
        //! Connection UrlType
        enum UrlType
        {
            OpenSimServer = 0,
            OpenSimGridServer,
            RealXtendAuthenticationServer
        };

        //! Performs login, should later return WorldStream etc (CAPS?)
        virtual bool StartSession(const LoginCredentials &credentials, const QUrl &serverEntryPointUrl) = 0;

        //! Make Url validation according to type
        virtual QUrl ValidateUrl(const QString &urlString, const UrlType urlType) = 0;

        //! Get login credentials
        virtual LoginCredentials GetCredentials() const = 0;

        //! Get server entry point url. Used for xmlrpc login_to_simulator and authentication internally.
        virtual QUrl GetServerEntryPointUrl() const = 0;

        //! Get created WorldStream: void -> WorldStreamInterface when implemented
        virtual void GetWorldStream() const = 0;

        //! Get connection thread state
        virtual ProtocolUtilities::ConnectionThreadState *GetConnectionThreadState() { return &threadState_; }

        //! Set login credentials
        virtual void SetCredentials(const LoginCredentials &credentials) = 0;

        //! Set server entry point url
        virtual void SetServerEntryPointUrl(const QUrl &newUrl) = 0;

    signals:
        /// Indicates that login process was succesful.
        void LoginSuccessful();

        /** Indicates that login process failed.
         *  @param message Error message.
         */
        void LoginFailed(const QString &message);

    private:
        //! State of the connection procedure thread.
        ProtocolUtilities::ConnectionThreadState threadState_;
    };
}

#endif // incl_RexLogic_WorldSessionInterface_h
