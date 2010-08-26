// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ClassicalLoginWidget.h"
#include "WebLoginWidget.h"
#include "TraditionalLoginWidget.h"

#include "Ether/EtherLoginNotifier.h"
#include "ui_ClassicalLoginWidget.h"

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    namespace Classical
    {
        ClassicalLoginWidget::ClassicalLoginWidget(
            Ether::Logic::EtherLoginNotifier *login_notifier,
            QMap<QString,QString> stored_login_data) :
            login_notifier_(login_notifier),
            traditional_widget_(new TraditionalLoginWidget(this, stored_login_data)),
            web_login_(new WebLoginWidget(this))
        {
            setupUi(this);
            tabWidget->addTab(traditional_widget_, " Login");
            tabWidget->addTab(web_login_, " Weblogin");

            connect(traditional_widget_, SIGNAL(Connect(const QMap<QString, QString> &)), login_notifier, SLOT(EmitLogin(const QMap<QString, QString> &)));
            connect(web_login_, SIGNAL(WebLoginInfoReceived(QWebFrame *)), login_notifier, SLOT( EmitLogin(QWebFrame *) ));
            connect(web_login_, SIGNAL(WebLoginUrlReceived(const QString &)), login_notifier, SLOT( EmitLogin(const QString &) ));

            connect(login_notifier, SIGNAL(LoginStarted()), SLOT(LoginStarted()));
            connect(login_notifier, SIGNAL(LoginFailed(const QString &)), SLOT(LoginFailed(const QString &)));
            connect(login_notifier, SIGNAL(LoginSuccessful()), SLOT(LoginSuccessful()));
        }

        ClassicalLoginWidget::~ClassicalLoginWidget()
        {
            SAFE_DELETE(traditional_widget_);
            SAFE_DELETE(web_login_);
        }

        void ClassicalLoginWidget::hideEvent(QHideEvent *hide_event)
        {
            QWidget::hideEvent(hide_event);
            emit ClassicLoginHidden();
        }

        void ClassicalLoginWidget::RemoveEtherButton()
        {
            traditional_widget_->RemoveEtherButton();
        }

        QMap<QString, QString> ClassicalLoginWidget::GetLoginInfo() const
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

        void ClassicalLoginWidget::LoginStarted()
        {
            traditional_widget_->StartProgressBar();
        }

        void ClassicalLoginWidget::LoginFailed(const QString &message)
        {
            traditional_widget_->SetStatus(message);
            traditional_widget_->StopProgressBar();
        }

        void ClassicalLoginWidget::LoginSuccessful()
        {
            traditional_widget_->SetStatus("Connected");
            traditional_widget_->StopProgressBar();
        }
    }
}
