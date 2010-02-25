// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_TraditionalLoginWidget_h
#define incl_UiModule_TraditionalLoginWidget_h

#include "AbstractLogin.h"

//#include "EventHandlers/LoginHandler.h"
#include "NetworkEvents.h"

#include "ui_TraditionalLoginWidget.h"

namespace Foundation
{
    class Framework;
}

namespace RexLogic
{
    class OpenSimLoginHandler;
}

namespace CoreUi
{
    class TraditionalLoginWidget : public AbstractLogin
    {
        Q_OBJECT

    public:
        TraditionalLoginWidget(LoginContainer *controller, RexLogic::OpenSimLoginHandler *os_login_handler, Foundation::Framework *framework);
        virtual ~TraditionalLoginWidget(void);

    public slots:
        void DoCommandParameterLogin(QMap<QString, QString> &login_information);

        //! Semi hack to get stuff into the demo world frame
        Ui::TraditionalLoginWidget GetUi() const { return ui_; }

    private:
        void InitWidget();
        void SetLoginHandler(RexLogic::OpenSimLoginHandler *os_login_handler);
        void ReadConfig();

        QString realxtend_username_;
        QString realxtend_server_;
        QString realxtend_authserver_;
        QString opensim_username_;
        QString opensim_server_;

        Foundation::Framework *framework_;
        Ui::TraditionalLoginWidget ui_;

    private slots:
        void ShowSelectedMode();
        void ParseInputAndConnect();

    signals:
        void Connecting();
        void ConnectOpenSim(QMap<QString, QString>);
        void ConnectRealXtend(QMap<QString, QString>);
    };
}

#endif // incl_UiModule_TraditionalLoginWidget_h
