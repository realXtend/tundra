// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoginContainer.h"

#include "WebLoginWidget.h"
#include "TraditionalLoginWidget.h"

#include "UiModule.h"
#include "UiSceneManager.h"
#include "UiProxyWidget.h"
#include "UiWidgetProperties.h"
#include "ModuleManager.h"
#include "Framework.h"

#include "../RexLogicModule/EventHandlers/LoginHandler.h"

#include <QUiLoader>
#include <QFile>
#include <QTimer>
#include <QGraphicsScene>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    LoginContainer::LoginContainer(Foundation::Framework *framework, RexLogic::OpenSimLoginHandler *os_login_handler, RexLogic::TaigaLoginHandler *taiga_login_handler) 
        : framework_(framework),
          os_login_handler_(os_login_handler),
          taiga_login_handler_(taiga_login_handler),
          login_widget_(new QWidget()),
          traditional_login_widget_(0),
          web_login_widget_(0),
          login_progress_widget_(0),
          login_progress_bar_(0),
          login_status_(0),
          progress_bar_timer_(0),
          autohide_timer_(0)
    {
        ui_services_ = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices);
        if (ui_services_.lock().get())
        {
            InitLoginUI(os_login_handler_, taiga_login_handler_);
            connect(this, SIGNAL( DisconnectSignal() ), os_login_handler_, SLOT( Logout() ));
        }
        else
        {
            UiServices::UiModule::LogDebug("CoreUi::LoginContainer >> Could not acquire UiServices module, skipping UI creation");
        }
    }

    LoginContainer::~LoginContainer()
    {
        SAFE_DELETE(login_widget_);
        traditional_login_widget_ = 0;
        web_login_widget_ = 0;
        login_proxy_widget_ = 0;
    }

    void LoginContainer::Connected()
    {
        login_is_in_progress_ = false;

        SAFE_DELETE(progress_bar_timer_);
        SAFE_DELETE(login_progress_widget_);
        login_progress_proxy_widget_ = 0;
    }

    void LoginContainer::QuitApplication()
    {
        emit QuitApplicationSignal();
    }

    void LoginContainer::AdjustConstraintsToSceneRect(const QRectF &rect)
    {
        if (login_widget_)
        {
            login_widget_->setMinimumWidth((int)rect.width());
            login_widget_->setMaximumWidth((int)rect.width());
        }
    }

    void LoginContainer::StartParameterLoginTaiga(QString &server_entry_point_url)
    {
        emit CommandParameterLogin(server_entry_point_url);
    }

    void LoginContainer::StartParameterLoginOpenSim(QString &first_and_last, QString &password, QString &server_address_with_port)
    {
        if (!server_address_with_port.startsWith("http://"))
            server_address_with_port = "http://" + server_address_with_port;

        QMap<QString, QString> map;
        map["AuthType"] = "OpenSim";
        map["Username"] = first_and_last;
        map["Password"] = password;
        map["WorldAddress"] = server_address_with_port;

        emit DisconnectSignal();
        emit CommandParameterLogin(map);
    }

    void LoginContainer::StartParameterLoginRealXtend(QString &username, QString &password, QString &authAddressWithPort, QString &server_address_with_port)
    {
        if (!server_address_with_port.startsWith("http://"))
            server_address_with_port = "http://" + server_address_with_port;
        if (!authAddressWithPort.startsWith("http://"))
            authAddressWithPort = "http://" + authAddressWithPort;

        QMap<QString, QString> map;
        map["AuthType"] = "RealXtend";
        map["Username"] = username;
        map["Password"] = password;
        map["WorldAddress"] = server_address_with_port;
        map["AuthenticationAddress"] = authAddressWithPort;

        emit DisconnectSignal();
        emit CommandParameterLogin(map);
    }

    void LoginContainer::InitLoginUI(RexLogic::OpenSimLoginHandler *os_login_handler, RexLogic::TaigaLoginHandler *taiga_login_handler)
    {
        boost::shared_ptr<UiServices::UiModule> uiServices = ui_services_.lock();
        if (uiServices.get())
        {
            ui_.setupUi(login_widget_);
            
            ui_.messageFrame->hide();
            ui_.messageLabel->setText("");
            ui_.autohideLabel->setText("");
            
            traditional_login_widget_ = new TraditionalLoginWidget(this, os_login_handler, framework_);
            web_login_widget_ = new WebLoginWidget(this, taiga_login_handler);

            ui_.tabWidget->clear();
            ui_.tabWidget->addTab(traditional_login_widget_, " Login"); // FIX stylesheets: tab text clipping without spaces :(
            ui_.tabWidget->addTab(web_login_widget_, " Web Login"); // FIX stylesheets: tab text clipping without spaces :(

            login_proxy_widget_ = uiServices->GetSceneManager()->AddWidgetToScene(login_widget_, UiServices::UiWidgetProperties("Login", UiServices::CoreLayoutWidget));
            
            if (login_proxy_widget_)
            {
                // Give UiModule this proxy for login/logout hide/show controlling
                uiServices->GetSceneManager()->SetLoginProxyWidget(login_proxy_widget_);
                // Keep all parts visible at any given time even if notify messages are given... kind of a hack but works
                AdjustConstraintsToSceneRect (login_proxy_widget_->scene()->sceneRect());
                connect(login_proxy_widget_->scene(), SIGNAL( sceneRectChanged(const QRectF &) ), this, SLOT( AdjustConstraintsToSceneRect(const QRectF &) ));
            }

            connect(ui_.hideButton, SIGNAL( clicked() ), this, SLOT( HideMessageFromUser() ));
            connect(this, SIGNAL( QuitApplicationSignal() ), os_login_handler, SLOT( Quit() ));
            login_is_in_progress_ = false;
        }
    }

    void LoginContainer::StartLoginProgressUI()
    {
        boost::shared_ptr<UiServices::UiModule> uiServices = ui_services_.lock();
        QUiLoader loader;
        QFile uiFile("./data/ui/login/login_progress.ui");

        if ( uiServices.get() && uiFile.exists() )
        {
            login_progress_widget_ = loader.load(&uiFile);
            login_progress_widget_->hide();
            login_status_ = login_progress_widget_->findChild<QLabel *>("statusLabel");
            login_progress_bar_ = login_progress_widget_->findChild<QProgressBar *>("progressBar");
            uiFile.close();

            login_progress_proxy_widget_ = uiServices->GetSceneManager()->AddWidgetToScene(
                login_progress_widget_, UiServices::UiWidgetProperties("Login loader", UiServices::CoreLayoutWidget));
            login_progress_proxy_widget_->show();
            login_is_in_progress_ = true;
        }
    }

    void LoginContainer::HideLoginProgressUI()
    {
        if (progress_bar_timer_)
        {
            progress_bar_timer_->stop();
            SAFE_DELETE(progress_bar_timer_);
        }

        SAFE_DELETE(login_progress_widget_);
        login_progress_proxy_widget_ = 0;
    }

    void LoginContainer::UpdateLoginProgressUI(const QString &status, int progress, const ProtocolUtilities::Connection::State connection_state)
    {
        if (connection_state != ProtocolUtilities::Connection::STATE_ENUM_COUNT)
        {
            if (connection_state == ProtocolUtilities::Connection::STATE_INIT_XMLRPC)
            {
                login_status_->setText("Initialising connection");
                AnimateProgressBar(5);
            }
            else if (connection_state == ProtocolUtilities::Connection::STATE_XMLRPC_AUTH_REPLY_RECEIVED)
            {
                login_status_->setText("Authentication reply received");
                AnimateProgressBar(13);
            }
            else if (connection_state == ProtocolUtilities::Connection::STATE_WAITING_FOR_XMLRPC_REPLY)
            {
                login_status_->setText("Waiting for servers response...");
                AnimateProgressBar(26);
            }
            else if (connection_state == ProtocolUtilities::Connection::STATE_XMLRPC_REPLY_RECEIVED)
            {
                login_status_->setText("Login response received");
                AnimateProgressBar(50);
            }
            else if (connection_state == ProtocolUtilities::Connection::STATE_INIT_UDP)
            {
                login_status_->setText("Creating World Stream...");
                AnimateProgressBar(50); 
            }
            CreateProgressTimer(250);
        }
        else
        {
            if (progress == 100)
            {
                progress_bar_timer_->stop();
                Connected();
                return;
            }
            login_status_->setText(status);
            AnimateProgressBar(progress);
            CreateProgressTimer(500);
        }
    }

    void LoginContainer::AnimateProgressBar(int new_value)
    {
        if (new_value > login_progress_bar_->value() && new_value != 100)
            for(int i=login_progress_bar_->value(); i<=new_value; i++)
                login_progress_bar_->setValue(i);
    }

    void LoginContainer::UpdateProgressBar()
    {
        if (!login_progress_bar_)
            return;

        int oldvalue = login_progress_bar_->value();
        if (oldvalue < 99)
        {
            if (oldvalue > 79 && progress_bar_timer_->interval() != 1000)
            {
                //we don't actually know here whether anything has happened in the net, so better not change the msg either
                //login_status_->setText("Downloading world objects...");
                CreateProgressTimer(1000); 
            }
            else if (oldvalue > 90 && progress_bar_timer_->interval() != 3500)
            {
                CreateProgressTimer(3500);
            }
            login_progress_bar_->setValue(++oldvalue); //shows % - which we actually don't know, 
        }
    }

    void LoginContainer::CreateProgressTimer(int interval)
    {
        if (progress_bar_timer_)
            progress_bar_timer_->stop();
        SAFE_DELETE(progress_bar_timer_);
        progress_bar_timer_ = new QTimer();
        QObject::connect(progress_bar_timer_, SIGNAL( timeout() ), this, SLOT( UpdateProgressBar() ));
        progress_bar_timer_->start(interval);
    }

    void LoginContainer::ShowMessageToUser(QString message, int autohide_seconds)
    {
        login_is_in_progress_ = false;

        HideLoginProgressUI();
        ui_.messageLabel->setText(message);
        ui_.messageFrame->show();

        if (autohide_timer_)
        {
            autohide_timer_->stop();
            SAFE_DELETE(autohide_timer_);
        }
        autohide_timer_ = new QTimer(this);
        QObject::connect(autohide_timer_, SIGNAL( timeout() ), this, SLOT( UpdateAutoHide() ));
        autohide_count_ = autohide_seconds;
        autohide_timer_->start(1000);
    }

    void LoginContainer::HideMessageFromUser()
    {
        autohide_timer_->stop();
        ui_.messageFrame->hide();
        ui_.messageLabel->setText("");
        ui_.autohideLabel->setText("");
        autohide_count_ = 0;
    }

    void LoginContainer::UpdateAutoHide()
    {
        if (autohide_count_ > 0)
        {
            ui_.autohideLabel->setText(QString("Autohide in %1").arg(QString::number(autohide_count_)));
            --autohide_count_;
        }
        else
            HideMessageFromUser();
    }
}
