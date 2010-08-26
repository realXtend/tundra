/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LoginHandler.h
 *  @brief  Performs login processes supported by Naali: OpenSim, RealXtend and Taiga weblogin.
 */

#ifndef incl_RexLogic_LoginHandler_h
#define incl_RexLogic_LoginHandler_h

#include "LoginCredentials.h"
#include "LoginServiceInterface.h"

namespace ProtocolUtilities
{
    class WorldSessionInterface;
}

namespace RexLogic
{
    class RexLogicModule;

    /// Performs login processes supported by Naali: OpenSim, RealXtend and Taiga weblogin.
    class LoginHandler : public Foundation::LoginServiceInterface
    {
        Q_OBJECT

    public:
        /** Constuctor.
         *  @param owner Owner module.
         */
        explicit LoginHandler(RexLogicModule *owner);

        /// Destructor.
        virtual ~LoginHandler();

    public slots:
        /// LoginServiceInterface override.
        void ProcessLoginData(const QMap<QString, QString> &data);

        /// LoginServiceInterface override.
        void ProcessLoginData(QWebFrame *frame);

        /// LoginServiceInterface override.
        void ProcessLoginData(const QString &url);

        /// LoginServiceInterface override.
        void StartWorldSession();

        /// LoginServiceInterface override.
        void Logout();

        /// LoginServiceInterface override.
        void Quit();

    private:
        /// World session.
        ProtocolUtilities::WorldSessionInterface *world_session_;

        /// Owner module.
        RexLogicModule *owner_;

        /// Login credentials.
        LoginCredentials credentials_;

        /// Server URL.
        QUrl server_entry_point_url_;

    private slots:
        void HandleLoginFailed(const QString &message);

        void HandleLoginSuccessful();
    };
}

#endif
