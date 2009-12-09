// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_LoginHelper_h
#define incl_Communication_LoginHelper_h

#include "MasterWidget.h"
#include "UiDefines.h"
#include "../ui_LoginWidget.h"

#include "../interface.h"

#include <QObject>

namespace UiHelpers
{
    class LoginHelper : public QObject
    {

    Q_OBJECT

    public:
        LoginHelper(Ui::LoginWidget login_ui);
        virtual ~LoginHelper();

    public slots:
        void TryLogin();
        void LoginCancelled();

        void ConnectionEstablished(Communication::ConnectionInterface &connection_interface);
        void ConnectionFailed(Communication::ConnectionInterface &connection_interface);
    
    private:
        Ui::LoginWidget login_ui_;

        Communication::CommunicationServiceInterface *communication_service_;
        Communication::ConnectionInterface *im_connection_;

    signals:
        void StateChange(UiState);

    };
}

#endif // incl_Communication_LoginHelper_h