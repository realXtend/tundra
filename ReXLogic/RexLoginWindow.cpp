// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "RexLoginWindow.h"
#include "RexLogicModule.h"
#include "QtModule.h"
//#include "GtkmmUI.h"
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QtUiTools>
#include <QTabWidget>
#include <QLineEdit>

#include <QGraphicsScene>
#include <QGraphicsView>

namespace RexLogic
{
    RexLoginWindow::RexLoginWindow(Foundation::Framework* framework, RexLogicModule *module) :
    framework_(framework), rex_logic_(module), login_widget_(0)
    {
        InitLoginWindow();
    }
    
    RexLoginWindow::~RexLoginWindow()
    {
        SAFE_DELETE(login_widget_);
    }

    void RexLoginWindow::InitLoginWindow()
    {
        Foundation::ModuleWeakPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule");

        // If this occurs, we're most probably operating in headless mode.
        if (qt_module.expired())
            return;

        QUiLoader loader;
        QFile file("./data/ui/login.ui");
        login_widget_ = loader.load(&file); 

        // Create connections.
        QPushButton *pButton = login_widget_->findChild<QPushButton *>("but_connect");
        QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Connect()));

        pButton = login_widget_->findChild<QPushButton *>("but_log_out");
        QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Disconnect()));

        pButton = login_widget_->findChild<QPushButton *>("but_quit");
        QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Quit()));       

        // Recover the connection settings that were used in previous login
        // from the xml configuration file.
        std::string strText = "";
        std::string strGroup = "Login";
        std::string strKey = "username";
             
        strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

        // Opensim username:
        QLineEdit *line = login_widget_->findChild<QLineEdit* >("line_user_name");
        line->setText(QString(strText.c_str()));

		strKey = "server";	
        strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

        // Opensim server: 
        line = login_widget_->findChild<QLineEdit* >("line_server");
        line->setText(QString(strText.c_str()));

		// Rex auth:
        line = login_widget_->findChild<QLineEdit* >("line_server_au");
        line->setText(QString(strText.c_str()));

		strKey = "auth_name";	
        strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

		// Rex auth: 
        line = login_widget_->findChild<QLineEdit* >("line_login_au");
        line->setText(QString(strText.c_str()));
         
		strKey = "auth_server";	
        strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);

		// Rex auth: 
        line = login_widget_->findChild<QLineEdit* >("line_auth_server");
        line->setText(QString(strText.c_str()));
         

        login_widget_->show();

        CreateLogoutMenu();
    }

    void RexLoginWindow::CreateLogoutMenu()
    {
        Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());

        // If this occurs, we're most probably operating in headless mode.
        if (!qt_ui)
            return;

        QGraphicsScene *scene = qt_ui->GetUIScene();

        ///\todo Free.
        logout_button_ = new QPushButton();
        logout_button_->setText("Log out");
        logout_button_->move(5, 5);
        scene->addWidget(logout_button_);
        QObject::connect(logout_button_, SIGNAL(clicked()), this, SLOT(DisconnectAndShowLoginWindow()));
        logout_button_->hide();

        ///\todo Free.
        quit_button_ = new QPushButton();
        quit_button_->setText("Quit");
        scene->addWidget(quit_button_);
        quit_button_->move(5, 30);
        QObject::connect(quit_button_, SIGNAL(clicked()), this, SLOT(Quit()));
        quit_button_->hide();
    }

    void RexLoginWindow::Connect()
    {
      // Check which tab is active     
      QTabWidget *widget = login_widget_->findChild<QTabWidget* >("tabWidget");
      int index = widget->currentIndex();
      
      bool successful = false;
      std::string user_name = "";
      std::string server_address = "";

      switch (index)
	{
	case 0:
	  {
	    // Open sim
	
	    QLineEdit *line = login_widget_->findChild<QLineEdit* >("line_user_name");
	    user_name = line->text().toStdString();
	    line =  login_widget_->findChild<QLineEdit* >("line_server");
	    server_address = line->text().toStdString();

	    // password

	    line = login_widget_->findChild<QLineEdit* >("line_password");

	    std::string password = line->text().toStdString();

	    successful = rex_logic_->GetServerConnection()->ConnectToServer(user_name,
									  password, server_address);
		if (successful)
      	{
			// Save login and server settings for future use. 
			framework_->GetConfigManager()->SetSetting<std::string>(
											std::string("Login"),
											std::string("server"),
											server_address);
			framework_->GetConfigManager()->SetSetting<std::string>(
											std::string("Login"),
											std::string("username"),
											user_name);
        }

	    break;
	  }
	case 1:
	  {
	    // Rex authentication 
	 
	    QLineEdit* line = 0;
	    user_name ="";
		line =  login_widget_->findChild<QLineEdit* >("line_server_au");
	    server_address = line->text().toStdString();

	    // password

	    line = login_widget_->findChild<QLineEdit* >("line_password_au");

	    std::string password = line->text().toStdString();
	   
	    line = login_widget_->findChild<QLineEdit* >("line_login_au");
	    std::string auth_login = line->text().toStdString();
		
		// START HACK because some reason authentication server needs a last name when system logs into localhost authentication server
		// we need to do this. 
		user_name = auth_login + " " + auth_login;
	    // END 
		
		line = login_widget_->findChild<QLineEdit* >("line_auth_server");
	    std::string auth_server = line->text().toStdString();
	 
	    successful = rex_logic_->GetServerConnection()->ConnectToServer(user_name, password,
									  server_address, auth_server, auth_login);
		// Because hack we clear this, just in case.
		user_name="";

		if (successful)
		{
			// Save login and server settings for future use. 
			framework_->GetConfigManager()->SetSetting<std::string>(
											std::string("Login"),
											std::string("auth_server"),
											auth_server);
			framework_->GetConfigManager()->SetSetting<std::string>(
											std::string("Login"),
											std::string("auth_name"),
											auth_login);
		}
	    
		break;
	  }
	case 2:
	  {
	    // OpenID
	    break;
	  }
	default:
	  break;
	}
      
     
    }

    void RexLoginWindow::DisconnectAndShowLoginWindow()
    {
        Disconnect();
        ShowLoginWindow();
    }

    void RexLoginWindow::HideLoginWindow()
    {
        login_widget_->hide();
        logout_button_->show();
        quit_button_->show();
    }

    void RexLoginWindow::ShowLoginWindow()
    {
        login_widget_->show();
        logout_button_->hide();
        quit_button_->hide();
    }

    void RexLoginWindow::Disconnect()
    {
        // Disconnect from server
        rex_logic_->GetServerConnection()->RequestLogout();
    }
  
    void RexLoginWindow::Quit()
    {   
        if (rex_logic_->GetServerConnection()->IsConnected())
            rex_logic_->GetServerConnection()->RequestLogout();
        
        framework_->Exit();      
    }
 
    void RexLoginWindow::UpdateConnectionStateToUI(OpenSimProtocol::Connection::State state)
    { 
        ///\todo Perhaps this state change should come from above instead of this function.
        if (state == OpenSimProtocol::Connection::STATE_CONNECTED)
            HideLoginWindow();
        else
            ShowLoginWindow();

        if (login_widget_ != 0)
        {
            QLabel *pLabel = login_widget_->findChild<QLabel* >("lab_state");
            pLabel->setText(QString(OpenSimProtocol::Connection::NetworkStateToString(state).c_str()));
        }
    }
}
