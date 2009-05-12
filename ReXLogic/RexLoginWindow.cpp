// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "RexLoginWindow.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    RexLoginWindow::RexLoginWindow(Foundation::Framework *framework, RexLogicModule *module) :
    framework_(framework), rexLogic_(module), loginWindow(0), entryServer(0), entryUsername(0)
    {
        InitLoginWindow();
    }
    
    RexLoginWindow::~RexLoginWindow()
    {
        SAFE_DELETE(loginWindow);
    }

    void RexLoginWindow::InitLoginWindow()
    {
        Foundation::ModuleWeakPtr gtkmmui_module = framework_->GetModuleManager()->GetModule("GtkmmUI");
        if (gtkmmui_module.expired() == false)
        {
            // Create the login window from glade (xml) file.
            loginControls = Gnome::Glade::Xml::create("data/loginWindow.glade");
        }

        if (!loginControls)
            return;
        
        // Get the widgets.
        loginControls->get_widget("dialog_login", loginWindow);
        entryUsername = loginControls->get_widget("entry_username",entryUsername);
        entryServer = loginControls->get_widget("entry_server", entryServer);
        
        // Bind callbacks.
        loginControls->connect_clicked("button_connect", sigc::mem_fun(*this, &RexLoginWindow::OnClickConnect));
        loginControls->connect_clicked("button_logout", sigc::mem_fun(*this, &RexLoginWindow::OnClickLogout));
        loginControls->connect_clicked("button_quit", sigc::mem_fun(*this, &RexLoginWindow::OnClickQuit));
        entryServer->signal_activate().connect(sigc::mem_fun(*this, &RexLoginWindow::OnClickConnect));
        
        // Read old connection settings from xml configuration file.
        std::string strText = "";
        std::string strGroup = "Login";
        std::string strKey = "server";
        
      
        strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);
        entryServer->set_text(Glib::ustring(strText));
   
        strKey = "username";
        strText = "";
        
        strText = framework_->GetDefaultConfigPtr()->GetSetting<std::string>(strGroup, strKey);
        entryUsername->set_text(Glib::ustring(strText));
        
        ///@note Pending : Currently password is not loaded and saved.
        
        // Set the window title and position.
        loginWindow->set_title("Login");
        loginWindow->set_position(Gtk::WIN_POS_CENTER);
        loginWindow->show();        
    }
    
    void RexLoginWindow::OnClickConnect()
    {
        // Initialize UI widgets.
        Gtk::Entry *entry_password = 0;
		Gtk::Entry *entry_authentication = 0;
		loginControls->get_widget("entry_password", entry_password);
		loginControls->get_widget("entry_authentication", entry_authentication);

		bool succesful = false;
		if (entry_authentication != 0 && std::string(entry_authentication->get_text()) != std::string(""))
		{
			// Connect to Authentication server.
		    // entry_authentication contains authentication server address and port
			// entry_auth_login contains login username. 
			
			Gtk::Entry *entry_auth_login = 0;
			loginControls->get_widget("entry_auth_login", entry_auth_login);
			
			succesful = rexLogic_->GetServerConnection()->ConnectToServer(entryUsername->get_text(), entry_password->get_text(),
			    entryServer->get_text(), entry_authentication->get_text(), entry_auth_login->get_text());
		}
		else 
		{
		    succesful = rexLogic_->GetServerConnection()->ConnectToServer(entryUsername->get_text(),
		        entry_password->get_text(), entryServer->get_text());
        }
        
		if (succesful)
		{
            // Save login and server settings for future use. 
            framework_->GetConfigManager()->SetSetting<std::string>(
                std::string("Login"),
                std::string("server"),
                std::string(entryServer->get_text()));
            framework_->GetConfigManager()->SetSetting<std::string>(
                std::string("Login"),
                std::string("username"),
                std::string(entryUsername->get_text()));
        }
    }
    
    void RexLoginWindow::OnClickLogout()
    {
        rexLogic_->GetServerConnection()->RequestLogout();
        ///\todo Handle server timeouts.
    }
    
    void RexLoginWindow::OnClickQuit()
    {
        if (rexLogic_->GetServerConnection()->IsConnected())
            rexLogic_->GetServerConnection()->RequestLogout();
        
        framework_->Exit();
    }

    void RexLoginWindow::UpdateConnectionStateToUI(OpenSimProtocol::Connection::State state)
    {
        Gtk::Label *label_state = 0;
        if (loginControls)
            loginControls->get_widget("label_state", label_state);

        if (label_state)
            label_state->set_text(OpenSimProtocol::Connection::NetworkStateToString(state));
    }
}
