// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_WebLoginWidget_h
#define incl_UiModule_WebLoginWidget_h

#include "UiModuleApi.h"
#include "AbstractLogin.h"

#include "NetworkEvents.h"

#include "ui_WebLoginWidget.h"
namespace RexLogic
{
    class TaigaLoginHandler;
}

namespace CoreUi
{
    class WebLoginWidget :  public AbstractLogin
    {
        Q_OBJECT

    public:
        WebLoginWidget(LoginContainer *controller, RexLogic::TaigaLoginHandler *taiga_login_handler);
        virtual ~WebLoginWidget(void);

    public slots:
        void GoToUrl();
        void GoToUrl(bool checked);
        void LoadStarted();
        void UpdateUi(int progress);
        void ProcessPage(bool success);

    private:
        void InitWidget();
        void ConnectSignals();
        void SetLoginHandler(RexLogic::TaigaLoginHandler *taiga_login_handler);

        Ui::WebLogin weblogin_ui_;
        QString address_;

    signals:
        void WebLoginInfoRecieved(QWebFrame *);
    };
}

#endif // incl_UiModule_WebLoginWidget_h
