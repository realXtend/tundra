// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ClassicalLoginWidget.h"
#include "WebLoginWidget.h"
#include "TraditionalLoginWidget.h"

#include "Ether/EtherLoginNotifier.h"
#include "ui_ClassicalLoginWidget.h"

namespace CoreUi
{
    namespace Classical
    {
        ClassicalLoginWidget::ClassicalLoginWidget(
            Ether::Logic::EtherLoginNotifier *login_notifier,
            QMap<QString,QString> stored_login_data) :
            QWidget(),
            login_notifier_(login_notifier),
            traditional_widget_(new TraditionalLoginWidget(this, stored_login_data)),
            web_login_(new WebLoginWidget(this))
        {
            setupUi(this);
            tabWidget->addTab(traditional_widget_, " Login");
            tabWidget->addTab(web_login_, " Weblogin");

            connect(web_login_, SIGNAL( WebLoginInfoRecieved(QWebFrame *) ),
                    login_notifier, SLOT( EmitTaigaLogin(QWebFrame *) ));
            connect(web_login_, SIGNAL( WebLoginUrlRecived(QString) ),
                    login_notifier, SLOT( EmitTaigaLogin(QString) ));
            connect(traditional_widget_, SIGNAL( ConnectOpenSim(QMap<QString, QString>) ),
                    login_notifier, SLOT( EmitOpenSimLogin(QMap<QString, QString>) ));
            connect(traditional_widget_, SIGNAL( ConnectRealXtend(QMap<QString, QString>) ),
                    login_notifier, SLOT( EmitRealXtendLogin(QMap<QString, QString>) ));
        }

        ClassicalLoginWidget::~ClassicalLoginWidget()
        {
            SAFE_DELETE(traditional_widget_);
            SAFE_DELETE(web_login_);
        }

        void ClassicalLoginWidget::RemoveEtherButton()
        {
            traditional_widget_->RemoveEtherButton();
        }

        QMap<QString, QString> ClassicalLoginWidget::GetLoginInfo()
        {
            return traditional_widget_->GetLoginInfo();
        }

        void ClassicalLoginWidget::AppExitRequest()
        {
            emit AppExitRequested();
        }

        void ClassicalLoginWidget::StatusUpdate(bool connecting, QString message)
        {
            traditional_widget_->StatusUpdate(connecting, message);
        }
    }
}
