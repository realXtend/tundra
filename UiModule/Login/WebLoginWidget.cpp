// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WebLoginWidget.h"
#include "LoginContainer.h"

namespace CoreUi
{

WebLoginWidget::WebLoginWidget(LoginContainer *controller, RexLogic::TaigaLoginHandler *taiga_login_handler) :
    AbstractLogin(controller),
    web_login_widget_(0)
{
    SetLayout();
    SetLoginHandler(taiga_login_handler);
    InitWidget();
}

WebLoginWidget::~WebLoginWidget()
{
    SAFE_DELETE(web_login_widget_);
    login_handler_ = 0;
}

void WebLoginWidget::SetLoginHandler(RexLogic::TaigaLoginHandler *taiga_login_handler)
{
    login_handler_ = taiga_login_handler;
    QObject::connect(controller_, SIGNAL( CommandParameterLogin(QString&) ), login_handler_, SLOT( ProcessCommandParameterLogin(QString&) ));
    QObject::connect(login_handler_, SIGNAL( LoginStarted() ), controller_, SLOT( StartLoginProgressUI() ));
}

void WebLoginWidget::InitWidget()
{
    QFile confFile("./data/default_login.ini");
    if (!confFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString url(confFile.readLine());
    confFile.close();
    web_login_widget_ = new WebLogin(this, url); 
    QObject::connect(web_login_widget_, SIGNAL( WebLoginInfoRecieved(QWebFrame *) ), login_handler_, SLOT( ProcessWebLogin(QWebFrame *) ));
    layout()->addWidget(web_login_widget_);
}

}
