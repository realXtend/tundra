// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QtModule.h"
#include "RexLogicModule.h"

#include <QtUiTools>
#include <QFile>
#include <QHash>
#include <QMessageBox>

#include "Login/LoginUI.h"

namespace RexLogic
{
    /////////////////////////////////////////
    // Login CLASS
    /////////////////////////////////////////

    Login::Login(Foundation::Framework *framework, RexLogicModule *rexLogicModule)
        : framework_(framework), rexLogicModule_(rexLogicModule)
    {
        InitUICanvas();
        InitLoginUI();
        InitLogoutUI();
    }

    Login::~Login(void)
    {    
		if (qtModule_.get() && canvas_login_.get() && canvas_logout_.get())
        {
            qtModule_->DeleteCanvas(canvas_login_->GetID());
            qtModule_->DeleteCanvas(canvas_logout_->GetID());
        }
    }

	void Login::Show()
	{
		if (canvas_login_.get())
			canvas_login_->Show();
	}

	void Login::Hide()
	{
		if (canvas_login_.get())
			canvas_login_->Hide();
	}

	void Login::QuitApplication()
	{
		if (rexLogicModule_->GetServerConnection()->IsConnected())
			rexLogicModule_->LogoutAndDeleteWorld();
		framework_->Exit();
	}

    void Login::Connected()
    {
        canvas_login_->Hide();
        canvas_logout_->Show();
        AdjustWindowSize(canvas_login_->GetRenderWindowSize());

        if (qtModule_.get())
            qtModule_->SetShowControlBar(true);
    }

	void Login::Disconnect()
	{
        if (qtModule_.get())
            qtModule_->SetShowControlBar(false);

		if (rexLogicModule_->GetServerConnection()->IsConnected())
        {
			rexLogicModule_->LogoutAndDeleteWorld();
            canvas_logout_->Hide();
            canvas_login_->Show();
            AdjustWindowSize(canvas_logout_->GetRenderWindowSize());
        }
	}

    void Login::InitUICanvas()
    {
        // Get QtModule and create canvas
        qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
        if (!qtModule_.get())
            return;
        canvas_login_ = qtModule_->CreateCanvas(QtUI::UICanvas::Internal).lock();
        canvas_logout_ = qtModule_->CreateCanvas(QtUI::UICanvas::Internal).lock();
    }

    void Login::InitLoginUI()
    {
        tabWidget_ = new QTabWidget(0);
        tabWidget_->addTab((QWidget*)new WebUI(tabWidget_, this, framework_, rexLogicModule_), QString("Web Login"));
        tabWidget_->addTab((QWidget*)new NaaliUI(tabWidget_, this, framework_, rexLogicModule_), QString("Traditional Login"));

        QSize rendererWindowSize = canvas_login_->GetRenderWindowSize();
        tabWidget_->resize(rendererWindowSize.width(), rendererWindowSize.height());
		canvas_login_->SetSize(rendererWindowSize.width(), rendererWindowSize.height());
        canvas_login_->SetPosition(0, 0);
        canvas_login_->AddWidget(tabWidget_);
        canvas_login_->SetAlwaysOnTop(true);
        canvas_login_->SetStationary(true);
        canvas_login_->SetResizable(false);
		canvas_login_->Show();

        QObject::connect(canvas_login_.get(), SIGNAL( RenderWindowSizeChanged(const QSize&) ), this, SLOT( AdjustWindowSize(const QSize&) ));
    }

    void Login::InitLogoutUI()
    {
        QUiLoader loader;
        QFile uiFile("./data/ui/inworld_controls.ui");

        if ( uiFile.exists() )
        {
            // Load ui to widget from file and get buttons
            QWidget *inworldControls = loader.load(&uiFile);
            inworldControls->resize(150, 25);
            logout_button_ = inworldControls->findChild<QPushButton *>("pushButton_Logout");
            quit_button_ = inworldControls->findChild<QPushButton *>("pushButton_Quit");
            uiFile.close();

            // Create UICanvas
            QSize parentWindowSize = canvas_logout_->GetRenderWindowSize();
            canvas_logout_->SetPosition(parentWindowSize.width()-95, 0);
            canvas_logout_->SetSize(95, 25);
            canvas_logout_->SetResizable(false);
            canvas_logout_->SetStationary(true);
            canvas_logout_->SetAlwaysOnTop(true);

            // Connect signals
            QObject::connect(canvas_logout_.get(), SIGNAL( RenderWindowSizeChanged(const QSize&) ), this, SLOT( AdjustWindowSize(const QSize&) ));
            QObject::connect(logout_button_, SIGNAL( clicked() ), this, SLOT( Disconnect() ));
            QObject::connect(quit_button_, SIGNAL( clicked() ), this, SLOT( QuitApplication() ));

            // Add widget to canvas and hide it as long as we are inworld
            canvas_logout_->AddWidget(inworldControls);
            canvas_logout_->Hide();
        }
    }

	void Login::AdjustWindowSize(const QSize &newSize)
	{
        if ( !canvas_login_->IsHidden() )
        {
		    canvas_login_->SetSize(newSize.width(), newSize.height());
            canvas_login_->SetPosition(0,0);
            canvas_login_->BringToTop();
            canvas_login_->Redraw();
        }
        
        if ( !canvas_logout_->IsHidden() )
        {
            canvas_logout_->SetPosition(newSize.width()-95, 0);
            canvas_logout_->BringToTop();
            canvas_logout_->Redraw();
        }
	}

    /////////////////////////////////////////
    // ABSTRACT AbstractLoginUI CLASS
    /////////////////////////////////////////
	
	AbstractLoginUI::AbstractLoginUI(QWidget *parent, Login *controller, Foundation::Framework* framework, RexLogicModule *rexLogic)
		: QWidget(parent), controller_(controller), framework_(framework), rexLogicModule_(rexLogic), loginHandler_(0)
	{
		
	}

	void AbstractLoginUI::SetLayout()
	{
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
	}

	void AbstractLoginUI::Show()
	{
		this->show();
	}

	void AbstractLoginUI::Hide()
	{
		this->Hide();
	}

	void AbstractLoginUI::LoginDone(bool success)
	{
        // Do something if needed, canvas hides/shows are already handled
	}

    /////////////////////////////////////////
    // NaaliUI CLASS
    /////////////////////////////////////////

	NaaliUI::NaaliUI(QWidget *parent, Login *controller, Foundation::Framework* framework, RexLogicModule *rexLogic)
		: AbstractLoginUI(parent, controller, framework, rexLogic)
	{
		SetLayout();
		SetLoginHandler();
		InitWidget();
		ReadConfig();
		ShowSelectedMode();
	}

	NaaliUI::~NaaliUI()
	{
		delete loginHandler_;
	}

	void NaaliUI::SetLoginHandler()
	{
		loginHandler_ = new OpenSimLoginHandler(framework_, rexLogicModule_);
		QObject::connect(loginHandler_, SIGNAL( LoginDone(bool) ), this, SLOT( LoginDone(bool) ));
	}

	void NaaliUI::InitWidget()
	{
		QUiLoader loader;
        QFile uiFile("./data/ui/login_new.ui");
		internalWidget_ = loader.load(&uiFile, this);
		uiFile.close();

		radioButton_openSim_ = findChild<QRadioButton *>("radioButton_OpenSim");
		radioButton_realXtend_ = findChild<QRadioButton *>("radioButton_realXtend");
		pushButton_connect_ = findChild<QPushButton *>("pushButton_Connect");
		pushButton_close_ = findChild<QPushButton *>("pushButton_Close");
		label_authAddress_ = findChild<QLabel *>("label_AuthenticationServer");
		lineEdit_authAddress_ = findChild<QLineEdit *>("lineEdit_AuthenticationAddress");
		lineEdit_worldAddress_ = findChild<QLineEdit *>("lineEdit_WorldAddress");
		lineEdit_username_ = findChild<QLineEdit *>("lineEdit_Username");
		lineEdit_password_ = findChild<QLineEdit *>("lineEdit_Password");

		QObject::connect(radioButton_openSim_, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
		QObject::connect(radioButton_realXtend_, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
		QObject::connect(pushButton_connect_, SIGNAL( clicked() ), this, SLOT( ParseInputAndConnect() ));
		QObject::connect(pushButton_close_, SIGNAL( clicked() ), controller_, SLOT( QuitApplication() ));
		QObject::connect(this, SIGNAL( ConnectOpenSim(QMap<QString, QString>) ), loginHandler_, SLOT( ProcessOpenSimLogin(QMap<QString, QString>) ));
		QObject::connect(this, SIGNAL( ConnectRealXtend(QMap<QString, QString>) ), loginHandler_, SLOT( ProcessRealXtendLogin(QMap<QString, QString>) ));

		this->layout()->addWidget(internalWidget_);
	}

	void NaaliUI::ReadConfig()
	{
		// Recover the connection settings that were used in previous login
		// from the xml configuration file.
		QString value, configKey;
		QString configGroup("Login");

		configKey = QString("username");
		opensim_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
		lineEdit_username_->setText(opensim_username_);

		configKey = QString("auth_name");
		realXtend_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());

	    configKey = QString("rex_server");
		realXtend_server_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());

		configKey = QString("server");
		value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
		lineEdit_worldAddress_->setText(value);
		opensim_server_ = value;

		configKey = QString("auth_server");
		value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
		lineEdit_authAddress_->setText(value);
		realXtend_authserver_ = value;
	}

	void NaaliUI::ShowSelectedMode()
	{
		bool hide = false;
		if (radioButton_openSim_->isChecked() == true)
		{
			hide = false;
			if (lineEdit_username_->text() == realXtend_username_)
			    lineEdit_username_->setText(opensim_username_);
            if (lineEdit_worldAddress_->text() == realXtend_server_)
			    lineEdit_worldAddress_->setText(opensim_server_);
		}
		else if (radioButton_realXtend_->isChecked() == true)
		{
			hide = true;
			if (lineEdit_username_->text() == opensim_username_)
			    lineEdit_username_->setText(realXtend_username_);
            if (lineEdit_worldAddress_->text() == opensim_server_)
			    lineEdit_worldAddress_->setText(realXtend_server_);
		}
		label_authAddress_->setVisible(hide);
		lineEdit_authAddress_->setVisible(hide);
        lineEdit_password_->clear();
	}

	void NaaliUI::ParseInputAndConnect()
	{
		if ( !lineEdit_worldAddress_->text().isEmpty() &&
			 !lineEdit_username_->text().isEmpty() &&
			 !lineEdit_password_->text().isEmpty() )
		{
			QMap<QString, QString> map;
			map["WorldAddress"] = lineEdit_worldAddress_->text();
			map["Username"] = lineEdit_username_->text();
			map["Password"] = lineEdit_password_->text();
			if (radioButton_openSim_->isChecked() == true)
			{
				emit( ConnectOpenSim(map) );
			}
			else if (radioButton_realXtend_->isChecked() == true && 
					 !lineEdit_authAddress_->text().isEmpty() )
			{
				map["AuthenticationAddress"] = lineEdit_authAddress_->text();
				emit( ConnectRealXtend(map) );
			}
		}
	}


    /////////////////////////////////////////
    // WebUI CLASS
    /////////////////////////////////////////

	WebUI::WebUI(QWidget *parent, Login *controller, Foundation::Framework *framework, RexLogicModule *rexLogic)
		: AbstractLoginUI(parent, controller, framework, rexLogic)
	{
		SetLayout();
		SetLoginHandler();
		InitWidget();
	}

	WebUI::~WebUI()
	{
		delete loginHandler_;
	}

	void WebUI::SetLoginHandler()
	{
		loginHandler_ = new TaigaLoginHandler(framework_, rexLogicModule_);
		QObject::connect(loginHandler_, SIGNAL( LoginDone(bool) ), this, SLOT( LoginDone(bool) ));
	}

	void WebUI::InitWidget()
	{
		QFile confFile("./data/default_login.ini");
		if (!confFile.open(QIODevice::ReadOnly | QIODevice::Text))
			return;
		QString url(confFile.readLine());
		confFile.close();
		webLogin_ = new WebLogin(this, url); 
		QObject::connect(webLogin_, SIGNAL( WebLoginInfoRecieved(QWebFrame *) ), loginHandler_, SLOT( ProcessWebLogin(QWebFrame *) ));
		this->layout()->addWidget(webLogin_);
	}

}