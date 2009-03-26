// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "NetTestLogicModule.h"
#include <Poco/ClassLibrary.h>

#include "RexProtocolMsgIDs.h"

using namespace RexTypes;

namespace NetTest
{
    NetTestLogicModule::NetTestLogicModule() 
    : ModuleInterface_Impl("NetTestLogicModule"),
    bRunning_(false),
    bLogoutSent_(false),
    loginWindow(0),
    netTestWindow(0)
    {
    }
    
    // virtual
    NetTestLogicModule::~NetTestLogicModule()
    {
    }

    // virtual
    void NetTestLogicModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void NetTestLogicModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    void NetTestLogicModule::Initialize()
    {
		using namespace OpenSimProtocol;
		///\todo weak_pointerize
        netInterface_ = dynamic_cast<OpenSimProtocolModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol));
		if (!netInterface_)
		{
			LogError("Getting network interface did not succeed.");
			///\todo Terminate this module.
			return;
		}
       
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual
    void NetTestLogicModule::PostInitialize()
    {
        //Get event category id's.
        inboundCategoryID_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        outboundCategoryID_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkOut");
        
        if(inboundCategoryID_ == 0 || outboundCategoryID_ == 0)
            LogWarning("Unable to find event category for OpenSimNetwork events!");
            
        InitLoginWindow();
        InitNetTestWindow();

        if(!loginWindow || !netTestWindow)
        {
            LogError("Could not initialize UI.");
            return;
        }
        
        loginWindow->set_position(Gtk::WIN_POS_CENTER);
        loginWindow->show();
    }

    // virtual 
    void NetTestLogicModule::Uninitialize()
    {
        SAFE_DELETE(netTestWindow)
        SAFE_DELETE(loginWindow)
        
        LogInfo("Module " + Name() + " uninitialized.");
    }

    // virtual
    void NetTestLogicModule::Update()
    {

    }

    //virtual 
    bool NetTestLogicModule::HandleEvent(
        Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        if (category_id == inboundCategoryID_)
        {
            OpenSimProtocol::NetworkEventInboundData *event_data = static_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
            const NetMsgID msgID = event_data->messageID;
            NetInMessage *msg = event_data->message;
            const NetMessageInfo *info = event_data->message->GetMessageInfo();
            assert(info);
            
            std::stringstream ss;
            ss << info->name << " received, " << Core::ToString(msg->GetDataSize()) << " bytes.";

		    LogInfo(ss.str());
		    if(bLogInbound_)
		        WriteToLogWindow(ss.str());
		        
            switch(msgID)
		    {
		    case RexNetMsgRegionHandshake:
			    {
				    LogInfo("\"RegionHandshake\" received, " + Core::ToString(msg->GetDataSize()) + " bytes.");

                    msg->SkipToNextVariable(); // RegionFlags U32
				    msg->SkipToNextVariable(); // SimAccess U8
				    size_t bytesRead = 0;
				    simName_ = (const char *)msg->ReadBuffer(&bytesRead);
    				
				    LogInfo("Joined to the sim \"" + simName_ + "\".");
    				
				    std::string title = "Logged in to ";
				    title.append(simName_);
                    netTestWindow->set_title(title);
    			    break;
			    }
		    case RexNetMsgChatFromSimulator:
		        {
		            std::stringstream ss;
		            size_t bytes_read;

		            std::string name = (const char *)msg->ReadBuffer(&bytes_read);
		            msg->SkipToFirstVariableByName("Message");
		            std::string message = (const char *)msg->ReadBuffer(&bytes_read);
		            ss << "[" << Core::GetLocalTimeString() << "] " << name << ": " << message << std::endl;

    	            WriteToChatWindow(ss.str());
		            break;
		        }
            case RexNetMsgLogoutReply:
			    {
				    RexUUID aID = msg->ReadUUID();
				    RexUUID sID = msg->ReadUUID();
    	
				    // Log out if the id's match.
				    if (aID == myInfo_.agentID && sID == myInfo_.sessionID)
				    {
					    LogInfo("\"LogoutReply\" received with matching IDs. Logging out.");
                        bRunning_ = false;
                        bLogoutSent_ = false;
                        netInterface_->DisconnectFromRexServer();
                        SAFE_DELETE(netTestWindow);
				    }
				    break;
			    }
		    default:
			    netInterface_->DumpNetworkMessage(msgID, msg);
			    break;
            }
        }
        else if (category_id == outboundCategoryID_)
        {
//            const msgID = event_data->message->messageID;
//            const NetInMessage *msg = event_data->message->message;
            OpenSimProtocol::NetworkEventOutboundData *event_data = static_cast<OpenSimProtocol::NetworkEventOutboundData *>(data);
            const NetMessageInfo *info = event_data->message->GetMessageInfo();
            assert(info);
            
            std::stringstream ss;
            ss << info->name << " sent, " << Core::ToString(event_data->message->BytesFilled()) << " bytes.";

		    LogInfo(ss.str());
		    if(bLogOutbound_)
		        WriteToLogWindow(ss.str());
        }

        return false;
    }

    void NetTestLogicModule::OnClickConnect()
    {
        if(bRunning_)
        {
            LogError("You are already connected to a server!");
            return;
        }
        
        // Initialize UI widgets.
        Gtk::Entry *entry_username;
        Gtk::Entry *entry_password;
        Gtk::Entry *entry_server;
        loginControls->get_widget("entry_username", entry_username);
        loginControls->get_widget("entry_password", entry_password);
        loginControls->get_widget("entry_server", entry_server);

        size_t pos;
        bool rex_login = false; ///\todo Implement rex-login.
        std::string username, first_name, last_name, password, server_address;
        int port;
        
        // Get username.
        if (!rex_login)
        {   
            
            username = entry_username->get_text();
            pos = username.find(" ");
            if(pos == std::string::npos)
            {
                LogError("Invalid username.");
                return;
            }
            first_name = username.substr(0, pos);
            last_name = username.substr(pos + 1);
        }
        else
            username = entry_username->get_text();
        
        // Get server address and port.
        std::string server = entry_server->get_text();
        pos = server.find(":");
        if(pos == std::string::npos)
        {
            LogError("Invalid syntax for server address and port. Use \"server:port\"");
            return;
        }
        
        server_address = server.substr(0, pos);
        try
        {
            port = boost::lexical_cast<int>(server.substr(pos + 1));
        } catch(std::exception)
        {
            LogError("Invalid port number, only numbers are allowed.");
            return;
        }
        
        // Get password.
        password = entry_password->get_text();
        
        bool success = netInterface_->ConnectToRexServer(first_name.c_str(), last_name.c_str(),
            password.c_str(), server_address.c_str(), port, &myInfo_);
            
        if(success)
        {
            if (!netTestWindow)
                InitNetTestWindow();
            
            netTestWindow->show();
            bRunning_ = true;
            SendUseCircuitCodePacket();
            SendCompleteAgentMovementPacket();
            
            LogInfo("Connected to server " + server_address + ".");
        }
        else
            LogError("Connecting to server " + server_address + " failed.");
    }
    
    void NetTestLogicModule::OnClickLogout()
    {
        if (bRunning_ && !bLogoutSent_)
        {
            SendLogoutRequestPacket();
            bLogoutSent_ = true;
        }
        ///\todo Handle server timeouts.
    }
    
    void NetTestLogicModule::OnClickQuit()
    {
        if (bRunning_ && !bLogoutSent_)
        {   
            SendLogoutRequestPacket();
            bLogoutSent_ = true;        
        }
        else
        {
            framework_->Exit();
            assert(framework_->IsExiting());        
        }
    }
    
    void NetTestLogicModule::OnClickChat()
    {
        Gtk::Entry *entry_chat = 0;
        netTestControls->get_widget("entry_chat", entry_chat);
        
        Glib::ustring text  = entry_chat->get_text();
        if(text == "")
            return;
        
        SendChatFromViewerPacket(text.c_str());
        entry_chat->set_text("");
    }
    
    void NetTestLogicModule::WriteToChatWindow(const std::string &message)
    {   
        // Get the widget controls.
        Gtk::ScrolledWindow *scrolledwindowChat = 0;
		Gtk::TextView *textviewChat = 0;
		netTestControls->get_widget("scrolledwindow_chat", scrolledwindowChat);
		netTestControls->get_widget("textview_chat", textviewChat);
		scrolledwindowChat->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
		
		// Create text buffer and write data to it.
		Glib::RefPtr<Gtk::TextBuffer> text_buffer = textviewChat->get_buffer();
        Gtk::TextBuffer::iterator iter = text_buffer->get_iter_at_offset(0);
		
		text_buffer->insert(iter, message);
        textviewChat->set_buffer(text_buffer);
    }
    
    void NetTestLogicModule::WriteToLogWindow(const std::string &message)
    {
        // Get the widget controls.
        Gtk::ScrolledWindow *scrolledwindowLog = 0;
		Gtk::TextView *textviewLog = 0;
		netTestControls->get_widget("scrolledwindow_log", scrolledwindowLog);
        netTestControls->get_widget("textview_log", textviewLog);
        scrolledwindowLog->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
		
		// Create text buffer and write data to it.
		Glib::RefPtr<Gtk::TextBuffer> text_buffer = textviewLog->get_buffer();
        Gtk::TextBuffer::iterator iter = text_buffer->get_iter_at_offset(0);
        
        std::stringstream output;
        output << "[" << Core::GetLocalTimeString() << "] " << message << std::endl;
        text_buffer->insert(iter, output.str());
        textviewLog->set_buffer(text_buffer);
    }
    
    void NetTestLogicModule::UpdateLogFilterState()
    {
        Gtk::CheckButton *checkbutton_inbound = netTestControls->get_widget("checkbutton_inbound", checkbutton_inbound);
        Gtk::CheckButton *checkbutton_outbound = netTestControls->get_widget("checkbutton_outbound", checkbutton_outbound);
        bLogInbound_ = checkbutton_inbound->get_active();
        bLogOutbound_ = checkbutton_outbound->get_active();
    }
    
	void NetTestLogicModule::SendUseCircuitCodePacket()
	{
		NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgUseCircuitCode);
		assert(m);
		m->AddU32(myInfo_.circuitCode);
		m->AddUUID(myInfo_.sessionID);
		m->AddUUID(myInfo_.agentID);
		netInterface_->FinishMessageBuilding(m);
	}

    void NetTestLogicModule::SendCompleteAgentMovementPacket()
    {
        NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgCompleteAgentMovement);
	    assert(m);
	    m->AddUUID(myInfo_.agentID);
	    m->AddUUID(myInfo_.sessionID);
	    m->AddU32(myInfo_.circuitCode);
	    netInterface_->FinishMessageBuilding(m);
    }

	void NetTestLogicModule::SendChatFromViewerPacket(const char *text)
	{
		NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgChatFromViewer);
		assert(m);
		m->AddUUID(myInfo_.agentID);
		m->AddUUID(myInfo_.sessionID);
		m->AddBuffer(strlen(text), (uint8_t*)text);
		m->AddU8(1);
		m->AddS32(0);
		netInterface_->FinishMessageBuilding(m);
	}
    
	void NetTestLogicModule::SendLogoutRequestPacket()
	{
		NetOutMessage *m = netInterface_->StartMessageBuilding(RexNetMsgLogoutRequest);
		assert(m);
    	m->AddUUID(myInfo_.agentID);
		m->AddUUID(myInfo_.sessionID);
	    netInterface_->FinishMessageBuilding(m);
	}

    void NetTestLogicModule::InitLoginWindow()
    {
        // Create the login window from glade (xml) file.
        loginControls = Gnome::Glade::Xml::create("data/loginWindow.glade");
        if (!loginControls)
            return;
        
        Gtk::Entry *entry_server = loginControls->get_widget("entry_server", entry_server);
        loginControls->get_widget("dialog_login", loginWindow);
        loginWindow->set_title("Login");
       
        // Bind callbacks.
        loginControls->connect_clicked("button_connect", sigc::mem_fun(*this, &NetTestLogicModule::OnClickConnect));
        loginControls->connect_clicked("button_logout", sigc::mem_fun(*this, &NetTestLogicModule::OnClickLogout));
        loginControls->connect_clicked("button_quit", sigc::mem_fun(*this, &NetTestLogicModule::OnClickQuit));
        entry_server->signal_activate().connect(sigc::mem_fun(*this, &NetTestLogicModule::OnClickConnect));
    }
    
    void NetTestLogicModule::InitNetTestWindow()
    {
        // Create the NetTest UI window from glade (xml) file.
        netTestControls = Gnome::Glade::Xml::create("data/NetTestWindow.glade");
        if (!netTestControls)
            return;
        
        UpdateLogFilterState();
        netTestControls->get_widget("window_nettest", netTestWindow);
        Gtk::Entry *entry_chat = netTestControls->get_widget("entry_chat", entry_chat);        
        netTestWindow->set_title("NetTest");
        
        // Bind callbacks.
        netTestControls->connect_clicked("button_chat", sigc::mem_fun(*this, &NetTestLogicModule::OnClickChat));
        netTestControls->connect_clicked("checkbutton_inbound", sigc::mem_fun(*this, &NetTestLogicModule::UpdateLogFilterState));
        netTestControls->connect_clicked("checkbutton_outbound", sigc::mem_fun(*this, &NetTestLogicModule::UpdateLogFilterState));        
        entry_chat->signal_activate().connect(sigc::mem_fun(*this, &NetTestLogicModule::OnClickChat));
    }
}

using namespace NetTest;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(NetTestLogicModule)
POCO_END_MANIFEST
