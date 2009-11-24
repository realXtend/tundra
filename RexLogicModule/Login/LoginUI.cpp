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
        : framework_(framework), rexLogicModule_(rexLogicModule), loginWidget_(0), loginProgressWidget_(0), inworldControls_(0), progressBarTimer_(0), autohideTimer_(0), loginProgressBar_(0), loginStatus_(0)
    {
        if ( InitUICanvases() )
        {
            InitLoginUI();
            InitLogoutUI();
        }
    }

    Login::~Login(void)
    {
		if (qtModule_.get() && canvas_login_.get() && canvas_logout_.get())
        {
            qtModule_->DeleteCanvas(canvas_login_->GetID());
            qtModule_->DeleteCanvas(canvas_logout_->GetID());
            if (canvas_login_progress_.get())
                qtModule_->DeleteCanvas(canvas_login_progress_->GetID());
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
        loginInProgress_ = false;
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

	void Login::StartCommandParameterLogin(QString &serverEntryPointUrl)
	{
		emit( CommandParameterLogin(serverEntryPointUrl) );
	}

    bool Login::InitUICanvases()
    {
        qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
        if (qtModule_.get())
        {
            canvas_login_ = qtModule_->CreateCanvas(QtUI::UICanvas::Internal).lock();
            canvas_logout_ = qtModule_->CreateCanvas(QtUI::UICanvas::Internal).lock();
            return true;
        }
        else
            return false;
        
    }

    void Login::InitLoginUI()
    {
        QUiLoader loader;
        QFile uiFile("./data/ui/login/login_controller.ui");

        if ( uiFile.exists() )
        {
            loginWidget_ = loader.load(&uiFile);
            
            messageFrame_ = loginWidget_->findChild<QFrame *>("messageFrame");
            hide_message_button_ = loginWidget_->findChild<QPushButton *>("hideButton");
            messageLabel_ = loginWidget_->findChild<QLabel *>("messageLabel");
            autohideLabel_ = loginWidget_->findChild<QLabel *>("autohideLabel");
            QTabWidget *tabWidget_ = loginWidget_->findChild<QTabWidget *>("tabWidget");
            
            messageFrame_->hide();
            messageLabel_->setText("");
            autohideLabel_->setText("");

            tabWidget_->clear();
            tabWidget_->addTab((QWidget*)new WebUI(tabWidget_, this, framework_, rexLogicModule_), QString("Web Login"));
            tabWidget_->addTab((QWidget*)new NaaliUI(tabWidget_, this, framework_, rexLogicModule_), QString("Traditional Login"));

            QSize rendererWindowSize = canvas_login_->GetRenderWindowSize();
            loginWidget_->resize(rendererWindowSize.width(), rendererWindowSize.height());
		    canvas_login_->SetSize(rendererWindowSize.width(), rendererWindowSize.height());
            canvas_login_->SetPosition(0, 0);
            canvas_login_->AddWidget(loginWidget_);
            canvas_login_->SetAlwaysOnTop(true);
            canvas_login_->SetStationary(true);
            canvas_login_->SetResizable(false);
		    canvas_login_->Show();

            loginInProgress_ = false;

            QObject::connect(canvas_login_.get(), SIGNAL( RenderWindowSizeChanged(const QSize&) ), this, SLOT( AdjustWindowSize(const QSize&) ));
            QObject::connect(hide_message_button_, SIGNAL( clicked() ), this, SLOT( HideMessageFromUser() ));
        }
    }

    void Login::InitLogoutUI()
    {
        QUiLoader loader;
        QFile uiFile("./data/ui/inworld_controls.ui");

        if ( uiFile.exists() )
        {
            // Load ui to widget from file and get buttons
            inworldControls_ = loader.load(&uiFile);
            inworldControls_->resize(150, 25);
            logout_button_ = inworldControls_->findChild<QPushButton *>("pushButton_Logout");
            quit_button_ = inworldControls_->findChild<QPushButton *>("pushButton_Quit");
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
            canvas_logout_->AddWidget(inworldControls_);
            canvas_logout_->Hide();

            loginInProgress_ = false;
        }
    }

    void Login::StartLoginProgressUI()
    {
        if (qtModule_.get())
        {
            canvas_login_progress_ = qtModule_->CreateCanvas(QtUI::UICanvas::Internal).lock();

            QUiLoader loader;
            QFile uiFile("./data/ui/login/login_progress.ui");

            if ( uiFile.exists() )
            {
                loginProgressWidget_ = loader.load(&uiFile);
                loginStatus_ = loginProgressWidget_->findChild<QLabel *>("statusLabel");
                loginProgressBar_ = loginProgressWidget_->findChild<QProgressBar *>("progressBar");
                uiFile.close();

                QSize parentWindowSize = canvas_login_progress_->GetRenderWindowSize();
                loginProgressWidget_->resize(parentWindowSize.width(), parentWindowSize.height());
                canvas_login_progress_->SetSize(parentWindowSize.width(), parentWindowSize.height());
                canvas_login_progress_->SetPosition(0, 0);
                canvas_login_progress_->SetResizable(false);
                canvas_login_progress_->SetStationary(true);
                canvas_login_progress_->SetAlwaysOnTop(true);
                canvas_login_progress_->AddWidget(loginProgressWidget_);
                canvas_login_progress_->Show();
                canvas_login_progress_->BringToTop();

                loginInProgress_ = true;
            }
        }
    }

    void Login::HideLoginProgressUI()
    {
        if (qtModule_.get())
        {
            if (progressBarTimer_)
            {
                progressBarTimer_->stop();
                SAFE_DELETE(progressBarTimer_);
            }
            SAFE_DELETE(loginProgressBar_);
            SAFE_DELETE(loginStatus_);

            if (canvas_login_progress_.get())
            {
                qtModule_->DeleteCanvas(canvas_login_progress_->GetID());
                canvas_login_progress_.reset();
            }
        }
    }

    void Login::UpdateLoginProgressUI(const QString &status, int progress, const ProtocolUtilities::Connection::State connectionState)
    {
        if (canvas_login_progress_.get())
        {
            if (connectionState != ProtocolUtilities::Connection::STATE_ENUM_COUNT)
            {
                if (connectionState == ProtocolUtilities::Connection::STATE_INIT_XMLRPC)
                {
                    loginStatus_->setText("Initialising connection");
                    AnimateProgressBar(5);
                }
                else if (connectionState == ProtocolUtilities::Connection::STATE_XMLRPC_AUTH_REPLY_RECEIVED)
                {
                    loginStatus_->setText("Authentication reply received");
                    AnimateProgressBar(13);
                }
                else if (connectionState == ProtocolUtilities::Connection::STATE_WAITING_FOR_XMLRPC_REPLY)
                {
                    loginStatus_->setText("Waiting for servers response...");
                    AnimateProgressBar(26);
                }
                else if (connectionState == ProtocolUtilities::Connection::STATE_XMLRPC_REPLY_RECEIVED)
                {
                    loginStatus_->setText("Login response received");
                    AnimateProgressBar(50);
                }
                else if (connectionState == ProtocolUtilities::Connection::STATE_INIT_UDP)
                {
                    loginStatus_->setText("Creating World Stream...");
                    AnimateProgressBar(50); 
                }
                CreateProgressTimer(250);
            }
            else
            {
                if (progress == 100 && qtModule_.get())
                {
                    progressBarTimer_->stop();
                    SAFE_DELETE(progressBarTimer_);
                    SAFE_DELETE(loginProgressBar_);
                    SAFE_DELETE(loginStatus_);

                    qtModule_->DeleteCanvas(canvas_login_progress_->GetID());
                    canvas_login_progress_.reset();

                    Connected();
                    return;
                }
                loginStatus_->setText(status);
                AnimateProgressBar(progress);
                CreateProgressTimer(500);
            }
        }
    }

    void Login::AnimateProgressBar(int newValue)
    {
        if (newValue > loginProgressBar_->value() && newValue != 100)
            for (int i=loginProgressBar_->value(); i<=newValue; i++)
                loginProgressBar_->setValue(i);
    }

    void Login::UpdateProgressBar()
    {
        if (loginProgressBar_)
        {
            int oldvalue = loginProgressBar_->value();
            if (oldvalue < 99)
            {
                if (oldvalue > 79 && progressBarTimer_->interval() != 1000)
                {
                    loginStatus_->setText("Downloading world objects...");
                    CreateProgressTimer(1000);
                }
                else if (oldvalue > 90 && progressBarTimer_->interval() != 3500)
                {
                    CreateProgressTimer(3500);
                }
                loginProgressBar_->setValue(++oldvalue);
            }
        }
    }

    void Login::CreateProgressTimer(int interval)
    {
        if (progressBarTimer_)
            progressBarTimer_->stop();
        SAFE_DELETE(progressBarTimer_);
        progressBarTimer_ = new QTimer();
        QObject::connect(progressBarTimer_, SIGNAL( timeout() ), this, SLOT( UpdateProgressBar() ));
        progressBarTimer_->start(interval);
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

        if ( loginInProgress_ )
        {
		    canvas_login_progress_->SetSize(newSize.width(), newSize.height());
            canvas_login_progress_->SetPosition(0,0);
            canvas_login_progress_->BringToTop();
            canvas_login_progress_->Redraw();
        }
	}

    void Login::ShowMessageToUser(QString message, int autohideSeconds)
    {
        HideLoginProgressUI();
        messageLabel_->setText(message);
        messageFrame_->show();

        if (autohideTimer_)
            autohideTimer_->stop();
        autohideTimer_ = new QTimer(this);
        QObject::connect(autohideTimer_, SIGNAL( timeout() ), this, SLOT( UpdateAutoHide() ));
        autoHideCount_ = autohideSeconds;
        autohideTimer_->start(1000);
    }

    void Login::HideMessageFromUser()
    {
        autohideTimer_->stop();
        messageFrame_->hide();
        messageLabel_->setText("");
        autohideLabel_->setText("");
        autoHideCount_ = 0;
    }

    void Login::UpdateAutoHide()
    {
        if ( autoHideCount_ > 0 )
        {
            autohideLabel_->setText(QString("Autohide in %1").arg(QString::number(autoHideCount_)));
            --autoHideCount_;
        }
        else
            HideMessageFromUser();
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
		setLayout(new QVBoxLayout(this));
		layout()->setMargin(0);
	}

	void AbstractLoginUI::LoginDone(bool success, QString &errorMessage)
	{
        // Does pretty much nothing, its always true due actual login 
        // is made in a thread and cant get return value straight away from it...
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
        QObject::connect(loginHandler_, SIGNAL( LoginStarted() ), controller_, SLOT( StartLoginProgressUI() ));
		QObject::connect(loginHandler_, SIGNAL( LoginDone(bool, QString&) ), this, SLOT( LoginDone(bool, QString&) ));
	}

	void NaaliUI::InitWidget()
	{
		QUiLoader loader;
        QFile uiFile("./data/ui/login/login_traditional.ui");

        if ( uiFile.exists() )
        {
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
        int fieldMissingCount = 0;
        QStringList missingFields;
        QString errorMessage;

		if ( !lineEdit_worldAddress_->text().isEmpty() &&
			 !lineEdit_username_->text().isEmpty() &&
			 !lineEdit_password_->text().isEmpty() )
		{
            controller_->UpdateLoginProgressUI(QString("Checking credentials"), 10, ProtocolUtilities::Connection::STATE_ENUM_COUNT);

			QMap<QString, QString> map;
			map["WorldAddress"] = lineEdit_worldAddress_->text();
			map["Username"] = lineEdit_username_->text();
			map["Password"] = lineEdit_password_->text();
			if (radioButton_openSim_->isChecked() == true)
			{
                if (map["Username"].count(" ") == 1 && !map["Username"].endsWith(" "))
				    emit( ConnectOpenSim(map) );
                else
                    controller_->ShowMessageToUser(QString("Your OpenSim username must be 'Firstname Lastname', you gave '%1'").arg(map["Username"]), 7);
                return;
			}
			else if (radioButton_realXtend_->isChecked() == true && 
					 !lineEdit_authAddress_->text().isEmpty() )
			{
				map["AuthenticationAddress"] = lineEdit_authAddress_->text();
				emit( ConnectRealXtend(map) );
                return;
			}
		}

        if (lineEdit_username_->text().isEmpty())
        {
            missingFields.append("Username");
        }
        if (lineEdit_password_->text().isEmpty())
        {
            missingFields.append("Password");
        }
        if (lineEdit_worldAddress_->text().isEmpty())
        {
            missingFields.append("World address");
        }
        if (lineEdit_authAddress_->text().isEmpty())
        {
            missingFields.append("Authentication address");
        }

        if (missingFields.count() >= 3)
        {
            errorMessage = QString("Please input the required fields ");
            int beforeAnd = missingFields.count() - 2;
            for (int i=0; i<beforeAnd; ++i)
            {
                errorMessage.append(missingFields.value(i));
                errorMessage.append(", ");
            }
            errorMessage.append(missingFields.value(beforeAnd));
            errorMessage.append(" and ");
            errorMessage.append(missingFields.value(++beforeAnd));
        }
        else if (missingFields.count() == 2)
        {
            errorMessage = QString("Please input the required fields ");
            errorMessage.append(missingFields.value(0));
            errorMessage.append(" and ");
            errorMessage.append(missingFields.value(1));
        }
        else if (missingFields.count() == 1)
        {
            errorMessage = QString("Please input the required field ");
            errorMessage.append(missingFields.value(0));
        }

        controller_->ShowMessageToUser(errorMessage, 7);
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
        delete webLogin_;
		delete loginHandler_;
	}

	void WebUI::SetLoginHandler()
	{
		loginHandler_ = new TaigaLoginHandler(framework_, rexLogicModule_);
		QObject::connect(controller_, SIGNAL( CommandParameterLogin(QString&) ), this, SLOT( DoCommandParameterLogin(QString&) ));
        QObject::connect(loginHandler_, SIGNAL( LoginStarted() ), controller_, SLOT( StartLoginProgressUI() ));
		QObject::connect(loginHandler_, SIGNAL( LoginDone(bool, QString&) ), this, SLOT( LoginDone(bool, QString&) ));
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

	void WebUI::DoCommandParameterLogin(QString &entryPointUrl)
	{
		static_cast<TaigaLoginHandler *>(loginHandler_)->ProcessCommandParameterLogin(entryPointUrl);
	}

}