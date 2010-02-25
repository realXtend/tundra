// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_LoginContainer_h
#define incl_UiModule_LoginContainer_h

#include "UiModuleApi.h"

//#include "EventHandlers/LoginHandler.h"
#include "NetworkEvents.h"

#include <QMap>
//#include <QProgressBar>

#include <QProgressBar>

#include "ui_LoginControllerWidget.h"

namespace Foundation
{
    class Framework;
}

namespace UiServices
{
    class UiModule;
    class UiProxyWidget;
}

namespace RexLogic
{
    class OpenSimLoginHandler;
    class TaigaLoginHandler;
}

namespace CoreUi
{
    class WebLoginWidget;
    class TraditionalLoginWidget;

    class UI_MODULE_API LoginContainer : public QObject
    {
        Q_OBJECT

    public:
        LoginContainer(Foundation::Framework *framework, RexLogic::OpenSimLoginHandler *os_login_handler, RexLogic::TaigaLoginHandler *taiga_login_handler);
        virtual ~LoginContainer();

        bool isLoginInProgress() const { return login_is_in_progress_; }

    public slots:
        void Connected();
        void QuitApplication();

        void AdjustConstraintsToSceneRect(const QRectF &rect);

        void StartParameterLoginTaiga(QString &server_entry_point_url);
        void StartParameterLoginOpenSim(QString &first_and_last, QString &password, QString &server_address_with_port);
        void StartParameterLoginRealXtend(QString &username, QString &password, QString &authAddressWithPort, QString &server_address_with_port);

        void StartLoginProgressUI();
        void HideLoginProgressUI();
        void UpdateLoginProgressUI(const QString &status, int progress, const ProtocolUtilities::Connection::State connection_state);

        void ShowMessageToUser(QString message, int autohide_seconds);

    signals:
        void CommandParameterLogin(QString&);
        void CommandParameterLogin(QMap<QString, QString>&);
        void QuitApplicationSignal();
        void DisconnectSignal();

    private:
        void InitLoginUI(RexLogic::OpenSimLoginHandler *os_login_handler, RexLogic::TaigaLoginHandler *taiga_login_handler);
        void CreateProgressTimer(int interval);

        Foundation::Framework *framework_;
        boost::weak_ptr<UiServices::UiModule> ui_services_;

        UiServices::UiProxyWidget *login_proxy_widget_;
        UiServices::UiProxyWidget *login_progress_proxy_widget_;
        Ui::LoginControllerWidget ui_;

        TraditionalLoginWidget *traditional_login_widget_;
        WebLoginWidget *web_login_widget_;

        QWidget *login_widget_;
        QWidget *login_progress_widget_;
        QLabel *login_status_;
        QProgressBar *login_progress_bar_;
        QTimer *progress_bar_timer_;
        QTimer *autohide_timer_;

        bool login_is_in_progress_;
        int autohide_count_;

        RexLogic::OpenSimLoginHandler *os_login_handler_;
        RexLogic::TaigaLoginHandler *taiga_login_handler_;

    private slots:
        void AnimateProgressBar(int new_value);
        void UpdateProgressBar();
        void UpdateAutoHide();
        void HideMessageFromUser();
    };
}

#endif
