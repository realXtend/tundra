// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AbstractLoginUI_h
#define incl_RexLogic_AbstractLoginUI_h

#include "StableHeaders.h"
#include "UICanvas.h"
#include "RexLogicModule.h"

#include <QtGui>
#include <QUiLoader>
#include <QFile>

#include "Login/LoginHandler.h"
#include "Login/WebLogin.h"

namespace QtUI
{
    class QtModule;
    class UICanvas;
}

namespace RexLogic 
{
    // ABSTRACT AbstractLoginUI CLASS

    class AbstractLoginUI : public QWidget
    {

    Q_OBJECT

	public:
		AbstractLoginUI(QWidget *parent, Login *controller, Foundation::Framework* framework, RexLogicModule *rexLogic);
		Login *controller_;
		AbstractLoginHandler *loginHandler_;
		Foundation::Framework *framework_;
		RexLogicModule *rexLogicModule_;
		virtual void InitWidget() = 0;        
        virtual void SetLoginHandler() = 0;
        virtual void SetLayout();
		virtual void Show();
        virtual void Hide();

	public slots:
		virtual void LoginDone(bool success);
		
    };

    // NaaliUI CLASS

	class Login;
    class NaaliUI : AbstractLoginUI
    {

	Q_OBJECT

    public:
        NaaliUI(QWidget *parent, Login *controller, Foundation::Framework* framework, RexLogicModule *rexLogic);
        virtual ~NaaliUI(void);

    private:
        void InitWidget();
		void SetLoginHandler();
		void ReadConfig();
		
		QString realXtend_username_;
		QString realXtend_server_;
		QString realXtend_authserver_;
		QString opensim_username_;
		QString opensim_server_;
		QWidget *internalWidget_;
		QRadioButton *radioButton_openSim_;
		QRadioButton *radioButton_realXtend_;
		QPushButton *pushButton_connect_;
		QPushButton *pushButton_close_;
		QLabel *label_authAddress_;
		QLineEdit *lineEdit_authAddress_;
		QLineEdit *lineEdit_worldAddress_;
		QLineEdit *lineEdit_username_;
		QLineEdit *lineEdit_password_;

	private slots:
		void ShowSelectedMode();
		void ParseInputAndConnect();

	signals:
		void ConnectOpenSim(QMap<QString, QString>);
		void ConnectRealXtend(QMap<QString, QString>);

    };

    // WebUI CLASS

    class WebUI : AbstractLoginUI
    {

	Q_OBJECT

    public:
        WebUI(QWidget *parent, Login *controller, Foundation::Framework* framework, RexLogicModule *rexLogic);
        virtual ~WebUI(void);

    private:
        void InitWidget();
		void SetLoginHandler();

		WebLogin *webLogin_;

    };

    // Login CLASS

    class Login : public QObject
    {

    Q_OBJECT

	public:
        Login(Foundation::Framework* framework, RexLogicModule *rexLogicModule);
        virtual ~Login(void);

    private:
        void InitUICanvas();
        void InitLoginUI();
        void InitLogoutUI();

        Foundation::Framework *framework_;
        RexLogicModule *rexLogicModule_;
        boost::shared_ptr<QtUI::QtModule> qtModule_;
        boost::shared_ptr<QtUI::UICanvas> canvas_login_;
        boost::shared_ptr<QtUI::UICanvas> canvas_logout_;
        
        QWidget *containerWidget_;
        QVBoxLayout *layout_;
        QTabWidget *tabWidget_;
        QPushButton *logout_button_;
        QPushButton *quit_button_;

	private slots:
		void AdjustWindowSize(const QSize &newSize);

	public slots:
		void Show();
		void Hide();
		void Disconnect();
        void Connected();
		void QuitApplication();

    };
}

#endif // incl_RexLogic_AbstractLoginUI_h