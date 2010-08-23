/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LoginServiceInterface.h
 *  @brief  Login service provides interface for passing your login information
 *          from login screen to the login logic.
 */

#ifndef incl_Interfaces_LoginServiceInterface_h
#define incl_Interfaces_LoginServiceInterface_h

#include "ServiceInterface.h"

#include <QObject>
#include <QMap>

class QWebFrame;

namespace Foundation
{
    /** Login service provides interface for passing your login information
     *  from login screen to the login logic.
     */
    class LoginServiceInterface : public QObject, public Foundation::ServiceInterface
    {
        Q_OBJECT

    public:
        /// Default constructor.
        LoginServiceInterface() {}

    public slots:
        /** Process login data and performs login procedure.
         *  @param data Login data map.
         */
        virtual void ProcessLoginData(const QMap<QString, QString> &data) = 0;

        /** This is an overloaded function.
         *  Process web login data.
         *  @frame Web frame
         */
        virtual void ProcessLoginData(QWebFrame *frame) = 0;

        /** This is an overloaded function.
         *  Process web login data.
         *  @url Web login URL.
         */
        virtual void ProcessLoginData(const QString &url) = 0;

        /// Starts world session i.e. creates UDP connection after succesfull login.
        virtual void StartWorldSession() = 0;

        /// Performs logout.
        virtual void Logout() = 0;

        /// @todo delete? Is it Loginhandler's responsibility to quit the program?
        virtual void Quit() = 0;

    signals:
        /// Indicates that login procedure has started.
        void LoginStarted();

        /** Indicates that login procedure has failed.
         *  @param message Error message.
         */
        void LoginFailed(const QString &message);

        /// Indicates that login was successful.
        void LoginSuccessful();

    private:
        Q_DISABLE_COPY(LoginServiceInterface);
    };
}

#endif
