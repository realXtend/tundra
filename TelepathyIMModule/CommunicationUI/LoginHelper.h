// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_LoginHelper_h
#define incl_Communication_LoginHelper_h

#include "ImUiDefines.h"
#include "ui_LoginWidget.h"

//#include "interface.h"
#include "TelepathyIMModuleFwd.h"

#include <QMap>

namespace UiHelpers
{
    class LoginHelper : public QObject
    {

    Q_OBJECT
    Q_PROPERTY(QString error_message_ READ GetErrorMessage)

    public:
        LoginHelper();
        virtual ~LoginHelper();

        //! Setters
        void SetupUi(Ui::LoginWidget *login_ui);

        //! Getters
        QString GetErrorMessage();
        QMap<QString,QString> GetPreviousCredentials();
        Communication::ConnectionInterface *GetConnectionInterface() { return im_connection_; }

    public slots:
        void TryLogin();
        void LoginCanceled();
        
        void ConnectionFailed(Communication::ConnectionInterface &connection_interface);
        void ConnectionEstablished(Communication::ConnectionInterface &connection_interface);
    
    private:
        Ui::LoginWidget *login_ui_;

        Communication::CommunicationServiceInterface *communication_service_;
        Communication::ConnectionInterface *im_connection_;

        QString error_message_;
        QString username_;
        QString server_;
        QString password_;

    signals:
        void StateChange(ImUiDefines::UiStates::ConnectionState);

    };
}

#endif // incl_Communication_LoginHelper_h