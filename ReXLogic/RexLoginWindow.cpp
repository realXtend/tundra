// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "RexLoginWindow.h"
#include "RexLogicModule.h"
//#include "GtkmmUI.h"
#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QtUiTools>
#include <QTabWidget>
#include <QLineEdit>

namespace RexLogic
{
    RexLoginWindow::RexLoginWindow(Foundation::Framework* framework, RexLogicModule *module) :
    framework_(framework), rexLogic_(module), pLogin_widget_(0)
    {
        InitLoginWindow();
    }
    
    RexLoginWindow::~RexLoginWindow()
    {
        SAFE_DELETE(pLogin_widget_);
    }

    void RexLoginWindow::InitLoginWindow()
    {
      ///todo Set check that qt_module exist 
      //Foundation::ModuleWeakPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule");
       
      //if (!qt_module.expired())
      //	 {
       QUiLoader loader;
       QFile file("../data/ui/login.ui");
       pLogin_widget_ = loader.load(&file); 
      
      //	 }
      //else
      // return;

       // Create connections.

       QPushButton *pButton = pLogin_widget_->findChild<QPushButton *>("but_connect");
       QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Connect()));

       pButton = pLogin_widget_->findChild<QPushButton *>("but_log_out");
       QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Disconnect()));
       
       pButton = pLogin_widget_->findChild<QPushButton *>("but_quit");
       QObject::connect(pButton, SIGNAL(clicked()), this, SLOT(Quit()));
       

       // Read old connection settings from xml configuration file.

       std::string strText = "";
       std::string strGroup = "Login";
       std::string strKey = "username";
       
      
       strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);
      
       // Open sim:

       QLineEdit *pLine = pLogin_widget_->findChild<QLineEdit* >("line_user_name");
       pLine->setText(QString(strText.c_str()));
       
       // Rex Auth: 

       pLine = pLogin_widget_->findChild<QLineEdit* >("line_user_name_au");
       pLine->setText(QString(strText.c_str()));
         
       strKey = "server";
       strText = "";
       
       strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);
      
       // Open sim: 

       pLine = pLogin_widget_->findChild<QLineEdit* >("line_server");
       pLine->setText(QString(strText.c_str()));
 
       // Rex Auth:

       pLine = pLogin_widget_->findChild<QLineEdit* >("line_server_au");
       pLine->setText(QString(strText.c_str()));
 
       // Show :
       pLogin_widget_->show();


    
    }

    void RexLoginWindow::Connect()
    {
      // Check which tab is active
      qDebug("RexLoginWindow::Connect() START");
      QTabWidget *pWidget = pLogin_widget_->findChild<QTabWidget* >("tabWidget");
      int index = pWidget->currentIndex();
      
      bool succesful = false;
      std::string user_name = "";
      std::string server_address = "";

      switch (index)
	{
	case 0:
	  {
	    // Open sim
	    qDebug("RexLoginWindow::Connect() Open Sim connection");
	    QLineEdit *pLine = pLogin_widget_->findChild<QLineEdit* >("line_user_name");
	    user_name = pLine->text().toStdString();
	    pLine =  pLogin_widget_->findChild<QLineEdit* >("line_server");
	    server_address = pLine->text().toStdString();

	    // password

	    pLine = pLogin_widget_->findChild<QLineEdit* >("line_password");

	    std::string password = pLine->text().toStdString();

	    succesful = rexLogic_->GetServerConnection()->ConnectToServer(user_name,
									  password, server_address);
	    break;
	  }
	case 1:
	  {
	    // Rex authentication 
	    qDebug("RexLoginWindow::Connect() Rex authentication connection");
	    QLineEdit *pLine = pLogin_widget_->findChild<QLineEdit* >("line_user_name_au");
	    user_name = pLine->text().toStdString();
	    pLine =  pLogin_widget_->findChild<QLineEdit* >("line_server_au");
	    server_address = pLine->text().toStdString();

	    // password

	    pLine = pLogin_widget_->findChild<QLineEdit* >("line_password_au");

	    std::string password = pLine->text().toStdString();
	    std::cout<<"Password: "<<password<<"user_name: "<<user_name<<" server_address: "<<server_address<<std::endl; 
	    pLine = pLogin_widget_->findChild<QLineEdit* >("line_auth_login");
	    std::string auth_login = pLine->text().toStdString();

	    pLine = pLogin_widget_->findChild<QLineEdit* >("line_auth_server");
	    std::string auth_server = pLine->text().toStdString();
	    std::cout<<"auth_server: "<<auth_server<<" auth_login: "<<auth_login<<std::endl;

	    succesful = rexLogic_->GetServerConnection()->ConnectToServer(user_name, password,
									  server_address, auth_server, auth_login);
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
      
      if (succesful)
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

     
    }

    void RexLoginWindow::Disconnect()
    {
      // Disconnect from server
      rexLogic_->GetServerConnection()->RequestLogout();
    }
  
    void RexLoginWindow::Quit()
    {
   
      if (rexLogic_->GetServerConnection()->IsConnected())
	rexLogic_->GetServerConnection()->RequestLogout();
        
      framework_->Exit();
      
    }
  
  
   void RexLoginWindow::UpdateConnectionStateToUI(OpenSimProtocol::Connection::State state)
    { 
      if ( pLogin_widget_ != 0)
	{
	  QLabel *pLabel = pLogin_widget_->findChild<QLabel* >("lab_state");
	  pLabel->setText(QString(OpenSimProtocol::Connection::NetworkStateToString(state).c_str()));

	}
      
      
    }
}
