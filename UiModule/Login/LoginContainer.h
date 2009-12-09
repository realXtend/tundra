// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_LoginUI_h
#define incl_UiModule_LoginUI_h

#include "StableHeaders.h"
#include "UiModuleApi.h"
#include "Login/WebLogin.h"

#include <EventHandlers/LoginHandler.h>
#include <NetworkEvents.h>

#include <QtGui>
#include <QUiLoader>
#include <QFile>

namespace UiServices
{
    class UiModule;
    class UiProxyWidget;
}

namespace CoreUi 
{
    class WebLoginWidget;
    class ClassicLoginWidget;

    class UI_MODULE_API LoginContainer : public QObject
    {

    Q_OBJECT

	public:
        LoginContainer(Foundation::Framework *framework, RexLogic::OpenSimLoginHandler *os_login_handler, RexLogic::TaigaLoginHandler *taiga_login_handler);
        virtual ~LoginContainer();

	public slots:
		void Connected();
        void QuitApplication();

		void StartParameterLoginTaiga(QString &server_entry_point_url);
        void StartParameterLoginOpenSim(QString &first_and_last, QString &password, QString &server_address_with_port);
        void StartParameterLoginRealXtend(QString &username, QString &password, QString &authAddressWithPort, QString &server_address_with_port);

        void StartLoginProgressUI();
        void HideLoginProgressUI();
        void UpdateLoginProgressUI(const QString &status, int progress, const ProtocolUtilities::Connection::State connection_state);

        void ShowMessageToUser(QString message, int autohide_seconds);

    private:
        void InitLoginUI(RexLogic::OpenSimLoginHandler *os_login_handler, RexLogic::TaigaLoginHandler *taiga_login_handler);
        void CreateProgressTimer(int interval);

        Foundation::Framework *framework_;
        boost::weak_ptr<UiServices::UiModule> ui_services_;

        UiServices::UiProxyWidget *login_proxy_widget_;
        UiServices::UiProxyWidget *login_progress_proxy_widget_;
        
        QWidget *login_progress_widget_;
        QWidget *login_widget_;
        QTabWidget *tabWidget_;
        ClassicLoginWidget *classic_login_widget_;
        WebLoginWidget *web_login_widget_;

        QFrame *message_frame_;
        QPushButton *logout_button_;
        QPushButton *quit_button_;

        QPushButton *hide_message_button_;
        QLabel *login_status_;
        QLabel *message_label_;
        QLabel *autohide_label_;
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

	signals:
		void CommandParameterLogin(QString&);
        void CommandParameterLogin(QMap<QString, QString>&);
        void QuitApplicationSignal();
        void DisconnectSignal();

    };
}

#endif // incl_UiModule_LoginUI_h
