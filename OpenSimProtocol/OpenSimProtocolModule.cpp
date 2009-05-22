// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimProtocolModule.h"
#include "RexProtocolMsgIDs.h"

namespace OpenSimProtocol
{
	OpenSimProtocolModule::OpenSimProtocolModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_OpenSimProtocol),
    connected_(false)
    {
    }
    
    // virtual
    OpenSimProtocolModule::~OpenSimProtocolModule()
    {
    }

    // virtual
    void OpenSimProtocolModule::Load()
    {
        LogInfo("System " + Name() + " loaded.");
    }

    // virtual
    void OpenSimProtocolModule::Unload()
    {
        LogInfo("System " + Name() + " unloaded.");
    }

    // virtual
    void OpenSimProtocolModule::Initialize()
    {
        ///\todo Read the template filename from a config file.
		const char *filename = "./data/message_template.msg";
		
		networkManager_ = boost::shared_ptr<NetMessageManager>(new NetMessageManager(filename));
		assert(networkManager_);
		    
		networkManager_->RegisterNetworkListener(this);
        
        // Register event categories.
        eventManager_ = framework_->GetEventManager();
        networkStateEventCategory_ = eventManager_->RegisterEventCategory("NetworkState");
        networkEventInCategory_ = eventManager_->RegisterEventCategory("OpenSimNetworkIn");
        networkEventOutCategory_ = eventManager_->RegisterEventCategory("OpenSimNetworkOut");

        LogInfo("System " + Name() + " initialized.");
    }

    // virtual 
    void OpenSimProtocolModule::Uninitialize()
    {		
		if(connected_)
		    DisconnectFromRexServer();
        
        networkManager_->UnregisterNetworkListener(this);
      
		LogInfo("System " + Name() + " uninitialized.");
    }
    
    // virtual
    void OpenSimProtocolModule::Update(Core::f64 frametime)
    {
        if (loginWorker_.IsReady() && loginWorker_.GetState() == Connection::STATE_XMLRPC_REPLY_RECEIVED)
        {
            // XML-RPC reply received; get the login parameters and signal that we're ready to
            // establish an UDP connection.
            clientParameters_ = loginWorker_.GetClientParameters();
            loginWorker_.SetConnectionState(Connection::STATE_INIT_UDP);
        }
        
        if (connected_)
            networkManager_->ProcessMessages();
    }
    
    //virtual
    void OpenSimProtocolModule::OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg)
    {
        // Send a Network event. The message ID functions as the event ID.
        NetworkEventInboundData data(msgID, msg);
        eventManager_->SendEvent(networkEventInCategory_, msgID, &data);
    }
        
    //virtual
    void OpenSimProtocolModule::OnNetworkMessageSent(const NetOutMessage *msg)
    {
        // Send a NetworkOutStats event.
        NetworkEventOutboundData data(msg->GetMessageID(), msg);
        eventManager_->SendEvent(networkEventOutCategory_, msg->GetMessageID(), &data);
    }

    void OpenSimProtocolModule::LoginToServer(
        const std::string& first_name,
		const std::string& last_name,
		const std::string& password,
		const std::string& address,
		int port,
		ConnectionThreadState *thread_state)
     {   
		std::string callMethod = "login_to_simulator";
		loginWorker_.SetupXMLRPCLogin(first_name, last_name, password, address, boost::lexical_cast<std::string>(port), callMethod, thread_state);

		// Start the thread.
		boost::thread(boost::ref(loginWorker_));
    }
    
    bool OpenSimProtocolModule::LoginUsingRexAuthentication(
        const std::string& first_name,
		const std::string& last_name,
		const std::string& password,
		const std::string& address,
		int port,
		const std::string& auth_server_address, 
		const std::string& auth_login,
		ConnectionThreadState *thread_state)
	{
		bool authentication = true;
		std::string callMethod = "ClientAuthentication";
		int pos = auth_server_address.find(":");
		std::string auth_port = "";
		std::string auth_address = "";
		
		if (pos != std::string::npos)
		{
			auth_port = auth_server_address.substr(pos+1);
			auth_address = auth_server_address.substr(0,pos);
		}
		else
		{
			LogError("Could not connect to server. Reason: port number was not found from authentication server address." );
            return false;
		}
        
        loginWorker_.SetupXMLRPCLogin(first_name, last_name, password, address, boost::lexical_cast<std::string>(port), callMethod,
            thread_state, auth_login, auth_address, auth_port, authentication);
        
        // Start the thread.
		boost::thread(boost::ref(loginWorker_));
        
		return true;
	}

    bool OpenSimProtocolModule::CreateUDPConnection(const char *address, int port)
	{
        loginWorker_.SetConnectionState(Connection::STATE_INIT_UDP);
	    
	    bool udp_success = networkManager_->ConnectTo(address, port);
        if (!udp_success)
            return false;
        
        loginWorker_.SetConnectionState(Connection::STATE_CONNECTED);
        
        // Send event indicating a succesfull connection.
        eventManager_->SendEvent(networkStateEventCategory_, Events::EVENT_SERVER_CONNECTED, NULL);
        connected_ = true;
        
        return true;	
	}
	
	void OpenSimProtocolModule::DisconnectFromRexServer()
	{
	    networkManager_->Disconnect();
	    loginWorker_.SetConnectionState(Connection::STATE_DISCONNECTED);
	    connected_ = false;
	}

	void OpenSimProtocolModule::DumpNetworkMessage(NetMsgID id, NetInMessage *msg)
	{
	    networkManager_->DumpNetworkMessage(id, msg);
	}
	
	NetOutMessage *OpenSimProtocolModule::StartMessageBuilding(NetMsgID msgId)
	{
	    return networkManager_->StartNewMessage(msgId);
	}
	
	void OpenSimProtocolModule::FinishMessageBuilding(NetOutMessage *msg)
	{
	    networkManager_->FinishMessage(msg);
	}
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace OpenSimProtocol;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OpenSimProtocolModule)
POCO_END_MANIFEST
