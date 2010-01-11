// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ClassicLoginWidget_h
#define incl_UiModule_ClassicLoginWidget_h

//#include "StableHeaders.h"
#include "AbstractLogin.h"

#include <EventHandlers/LoginHandler.h>
#include <NetworkEvents.h>

#include <QtGui>
#include <QUiLoader>
#include <QFile>

namespace CoreUi
{
    class ClassicLoginWidget : public AbstractLogin
    {
        Q_OBJECT

    public:
        ClassicLoginWidget(LoginContainer *controller, RexLogic::OpenSimLoginHandler *os_login_handler, Foundation::Framework *framework);
        virtual ~ClassicLoginWidget(void);

    public slots:
        void DoCommandParameterLogin(QMap<QString, QString> &login_information);

    private:
        void InitWidget();
        void SetLoginHandler(RexLogic::OpenSimLoginHandler *os_login_handler);
        void ReadConfig();

        QString realxtend_username_;
        QString realxtend_server_;
        QString realxtend_authserver_;
        QString opensim_username_;
        QString opensim_server_;
        QWidget *internal_widget_;
        QRadioButton *radiobutton_opensim_;
        QRadioButton *radiobutton_realxtend_;
        QPushButton *pushbutton_connect_;
        QPushButton *pushbutton_close_;
        QLabel *label_auth_address_;
        QLineEdit *line_edit_auth_address_;
        QLineEdit *line_edit_world_address_;
        QLineEdit *line_edit_username_;
        QLineEdit *line_edit_password_;

        Foundation::Framework *framework_;

    private slots:
        void ShowSelectedMode();
        void ParseInputAndConnect();

    signals:
        void Connecting();
        void ConnectOpenSim(QMap<QString, QString>);
        void ConnectRealXtend(QMap<QString, QString>);

    };
}

#endif // incl_UiModule_ClassicLoginWidget_h
