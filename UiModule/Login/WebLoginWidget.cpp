// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WebLoginWidget.h"
#include "LoginContainer.h"
#include "EventHandlers/LoginHandler.h"

#include <QFile>
#include <QLineEdit>
#include <QWebFrame>

namespace CoreUi
{
    WebLoginWidget::WebLoginWidget(LoginContainer *controller, RexLogic::TaigaLoginHandler *taiga_login_handler) 
        : AbstractLogin(controller)
    {
        SetLoginHandler(taiga_login_handler);
        InitWidget();
        ConnectSignals();
    }

    WebLoginWidget::~WebLoginWidget()
    {
        login_handler_ = 0;
    }

    void WebLoginWidget::SetLoginHandler(RexLogic::TaigaLoginHandler *taiga_login_handler)
    {
        login_handler_ = taiga_login_handler;
        connect(controller_, SIGNAL( CommandParameterLogin(QString&) ), login_handler_, SLOT( ProcessCommandParameterLogin(QString&) ));
        connect(login_handler_, SIGNAL( LoginStarted() ), controller_, SLOT( StartLoginProgressUI() ));
    }

    void WebLoginWidget::InitWidget()
    {
        // Read default url from file
        QFile confFile("./data/default_login.ini");
        if (!confFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        address_ = confFile.readLine();
        confFile.close();

        // Init UI
        weblogin_ui_.setupUi(this);

        weblogin_ui_.comboBox_Address->setEditText(address_);
        
        weblogin_ui_.pushButton_Back->setIcon(QIcon("./data/ui/images/arrow_left_48.png"));
        weblogin_ui_.pushButton_Back->setIconSize(QSize(20, 20));
        weblogin_ui_.pushButton_Forward->setIcon(QIcon("./data/ui/images/arrow_right_48.png"));
        weblogin_ui_.pushButton_Forward->setIconSize(QSize(20, 20));
        weblogin_ui_.pushButton_Stop->setIcon(QIcon("./data/ui/images/cross_48.png"));
        weblogin_ui_.pushButton_Stop->setIconSize(QSize(20, 20));
        weblogin_ui_.pushButton_Stop->setEnabled(false);
        weblogin_ui_.pushButton_Refresh->setIcon(QIcon("./data/ui/images/refresh_48.png"));
        weblogin_ui_.pushButton_Refresh->setIconSize(QSize(20, 20));
        weblogin_ui_.pushButton_Go->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
        weblogin_ui_.pushButton_Go->setIconSize(QSize(20, 20));
    }

    void WebLoginWidget::ConnectSignals()
    {
        // Buttons
        connect(weblogin_ui_.pushButton_Back, SIGNAL( clicked() ), weblogin_ui_.webView, SLOT( back() ));
        connect(weblogin_ui_.pushButton_Forward, SIGNAL( clicked() ), weblogin_ui_.webView, SLOT( forward() ));
        connect(weblogin_ui_.pushButton_Stop, SIGNAL( clicked() ), weblogin_ui_.webView, SLOT( stop() ));
        connect(weblogin_ui_.pushButton_Refresh, SIGNAL( clicked() ), weblogin_ui_.webView, SLOT( reload() ));
        connect(weblogin_ui_.pushButton_Go, SIGNAL( clicked(bool) ), this, SLOT( GoToUrl(bool) ));
        
        // Addressbar
        connect(weblogin_ui_.comboBox_Address->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( GoToUrl() ));
        
        // Webview
        connect(weblogin_ui_.webView, SIGNAL( loadStarted() ), this, SLOT( LoadStarted() ));
        connect(weblogin_ui_.webView, SIGNAL( loadProgress(int) ), this, SLOT( UpdateUi(int) ));
        connect(weblogin_ui_.webView, SIGNAL( loadFinished(bool) ), this, SLOT( ProcessPage(bool) ));
        connect(this, SIGNAL( WebLoginInfoRecieved(QWebFrame *) ), login_handler_, SLOT( ProcessWebLogin(QWebFrame *) ));

        weblogin_ui_.webView->setUrl(QUrl(address_));
    }

    void WebLoginWidget::GoToUrl()
    {
        GoToUrl(true);
    }

    void WebLoginWidget::GoToUrl(bool checked)
    {
        weblogin_ui_.webView->setUrl(QUrl(weblogin_ui_.comboBox_Address->lineEdit()->text()));
    }

    void WebLoginWidget::LoadStarted()
    {
        weblogin_ui_.pushButton_Stop->setEnabled(true);
        weblogin_ui_.label_Status->setText("Loading page...");
        weblogin_ui_.progressBar_Status->show();
    }

    void WebLoginWidget::UpdateUi(int progress)
    {
        if (weblogin_ui_.progressBar_Status)
            weblogin_ui_.progressBar_Status->setValue(progress);
    }

    void WebLoginWidget::ProcessPage(bool success)
    {
        if (success)
        {
            // Update GUI
            weblogin_ui_.pushButton_Stop->setEnabled(false);
            address_ = weblogin_ui_.webView->url().toString();
            weblogin_ui_.comboBox_Address->lineEdit()->setText(address_);
            //setWindowTitle(webView_->page()->mainFrame()->title().append(" - realXtend Naali web browser"));
            if (weblogin_ui_.comboBox_Address->findText(address_, Qt::MatchFixedString) == -1)
                weblogin_ui_.comboBox_Address->addItem(address_);
            weblogin_ui_.label_Status->setText("Done");
            weblogin_ui_.progressBar_Status->hide();

            // Do actual HTML page processing if this was a login success page...
            if (weblogin_ui_.webView->page()->mainFrame()->title() == "LoginSuccess")
                emit WebLoginInfoRecieved(weblogin_ui_.webView->page()->mainFrame());
        }
    }
}
