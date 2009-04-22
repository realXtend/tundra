// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimProtocolModule.h"
#include <Poco/ClassLibrary.h>
#include "ComponentRegistrarInterface.h"

#include "md5wrapper.h"
#include "PocoXMLRPC.h"
#include "OpenSimAuth.h"
#include "RexProtocolMsgIDs.h"

using namespace Foundation;

namespace OpenSimProtocol
{
	OpenSimProtocolModule::OpenSimProtocolModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_OpenSimProtocol),
    bConnected_(false)
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
        ///\todo Read the template filename from a config file?
		const char *filename = "./data/message_template.msg";
		
		networkManager_ = shared_ptr<NetMessageManager>(new NetMessageManager(filename));
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
		if(bConnected_)
		    DisconnectFromRexServer();
        
        networkManager_->UnregisterNetworkListener(this);
      
		LogInfo("System " + Name() + " uninitialized.");
    }

    // virtual
    void OpenSimProtocolModule::Update(Core::f64 frametime)
    {
        if (bConnected_)
            networkManager_->ProcessMessages();
    }
    
    //virtual
    void OpenSimProtocolModule::OnNetworkMessageReceived(NetMsgID msgID, NetInMessage *msg)
    {
        // Send a Network event.
        assert(msg);
        NetworkEventInboundData data(msgID, msg);
        eventManager_->SendEvent(networkEventInCategory_, msgID, &data);
    }
        
    //virtual
    void OpenSimProtocolModule::OnNetworkMessageSent(const NetOutMessage *msg)
    {
        // Send a NetworkOutStats event.
        assert(msg);
        NetworkEventOutboundData data(msg->GetMessageID(), msg);
        eventManager_->SendEvent(networkEventOutCategory_, msg->GetMessageID(), &data);
    }
    
   bool OpenSimProtocolModule::ConnectToRexServer(
				const std::string& first_name,
				const std::string& last_name,
				const std::string& password,
				const std::string& address,
				int port)
        
	{
		std::string callMethod = "login_to_simulator";
		bool xmlrpc_success = PerformXMLRPCLogin(first_name, last_name, password, address, boost::lexical_cast<std::string>(port), callMethod);
	    if (!xmlrpc_success)
	        return false;

	    bool network_success = networkManager_->ConnectTo(address.c_str(), port);
        if (!network_success)
            return false;

        eventManager_->SendEvent(networkStateEventCategory_, Events::EVENT_SERVER_CONNECTED, NULL);
        bConnected_ = true;
        
        return true;
	}

   bool OpenSimProtocolModule::ConnectUsingAuthenticationServer(const std::string& first_name,
			const std::string& last_name,
			const std::string& password,
			const std::string& address,
			int port,
			const std::string& auth_server_address, 
			const std::string& auth_login)
	{
		
		bool authentication = true;
		std::string callMethod = "ClientAuthentication";
		int pos = auth_server_address.find(":");
		std::string auth_port = "";
		std::string auth_address = "";
		
		if ( pos != std::string::npos)
		{
			auth_port = auth_server_address.substr(pos+1);
			auth_address = auth_server_address.substr(0,pos);
		}
		else
		{
			LogError("Could not connect to server. Reason: port number was not found from authentication server address." );
            return false;
		}
		bool xmlrpc_success = PerformXMLRPCLogin(first_name, last_name, password, address, boost::lexical_cast<std::string>(port), callMethod, 
			auth_login, auth_address, auth_port, authentication);
		
		
		if ( !xmlrpc_success)
			return false;
		//else
		//	eventManager_->SendEvent(networkStateEventCategory_, Events::EVENT_AUTHENTICATION_SUCCESS, NULL);

		callMethod = "login_to_simulator";

		xmlrpc_success = PerformXMLRPCLogin(first_name, last_name, password, address, boost::lexical_cast<std::string>(port), callMethod, 
			auth_login, auth_address, auth_port,authentication);
	
		if ( !xmlrpc_success)
			return false;

		bool network_success = networkManager_->ConnectTo(address.c_str(), port);
        if (!network_success)
            return false;

        eventManager_->SendEvent(networkStateEventCategory_, Events::EVENT_SERVER_CONNECTED, NULL);
        bConnected_ = true;

		
		return true;
	}
	
	void OpenSimProtocolModule::DisconnectFromRexServer()
	{
	    networkManager_->Disconnect();
	    bConnected_ = false;
	}
	
   bool OpenSimProtocolModule::PerformXMLRPCLogin(const std::string& first_name, 
				const std::string& last_name, 
				const std::string& password,
				const std::string& worldAddress,
				const std::string& worldPort,
				const std::string& callMethod,
				const std::string& authentication_login,
				const std::string& authentication_address,
				const std::string& authentication_port,
				bool authentication)
    {
		// create a MD5 hash for the password, MAC address and HDD serial number.
		std::string mac_addr = GetMACaddressString();
		std::string id0 = GetId0String();

		md5wrapper md5;
		std::string password_hash = "$1$" + md5.getHashFromString(password);
		std::string mac_hash = md5.getHashFromString(mac_addr);
		std::string id0_hash = md5.getHashFromString(id0);

        try
        {
			if ( authentication && callMethod != std::string("login_to_simulator") )
				rpcConnection_ = shared_ptr<PocoXMLRPCConnection>(new PocoXMLRPCConnection(authentication_address.c_str(), boost::lexical_cast<int>(authentication_port)));
			else if ( callMethod == std::string("login_to_simulator"))
				rpcConnection_ = shared_ptr<PocoXMLRPCConnection>(new PocoXMLRPCConnection(worldAddress.c_str(), boost::lexical_cast<int>(worldPort)));
        } catch(std::exception &e)
        {
            LogError("Could not connect to server. Reason: " + Core::ToString(e.what()) + ".");
            return false;
        }

		boost::shared_ptr<PocoXMLRPCCall> call = rpcConnection_->StartXMLRPCCall(callMethod.c_str());
		
		if(!call)
            return false;
            
		if ( !authentication && callMethod == std::string("login_to_simulator"))
		{
			call->AddStringMember("first", first_name.c_str());
			call->AddStringMember("last", last_name.c_str());
			call->AddStringMember("passwd", password_hash.c_str());
	     }
		else if ( authentication && callMethod == std::string("ClientAuthentication") )
		{
			std::string account = authentication_login + "@" + authentication_address + ":" +authentication_port; 
			call->AddStringMember("account", account.c_str());
			call->AddStringMember("passwd", password_hash.c_str());
			std::string loginuri = "";
		
			loginuri = loginuri+worldAddress+":"+ worldPort;
			call->AddStringMember("loginuri", loginuri.c_str());
		}
		else if ( authentication && callMethod == std::string("login_to_simulator"))
		{

			call->AddStringMember("sessionhash",clientParameters_.sessionHash.c_str());
			std::string account = authentication_login + "@" + authentication_address + ":" +authentication_port; 
			call->AddStringMember("account", account.c_str());
			std::string address = authentication_address + ":" + authentication_port;
			call->AddStringMember("AuthenticationAddress", address.c_str());
			std::string loginuri = "";
			if ( !worldAddress.find("http") != std::string::npos )
				loginuri = "http://";
			
			loginuri = loginuri + worldAddress+":"+ worldPort;
			call->AddStringMember("loginuri", loginuri.c_str());

		}

		call->AddStringMember("start", "last"); // Starting position perhaps?
	    call->AddStringMember("version", "realXtend 1.20.13.91224");  ///\todo Make build system create versioning information.
		call->AddStringMember("channel", "realXtend");
		call->AddStringMember("platform", "Win"); ///\todo.
		call->AddStringMember("mac", mac_hash.c_str());
		call->AddStringMember("id0", id0_hash.c_str());
		call->AddIntMember("last_exec_event", 0); // ?


		// The contents of 'options' array unknown. ///\todo Go through them and identify what they really affect.
		PocoXMLRPCCall::StringArray arr = call->CreateStringArray("options");
		call->AddStringToArray(arr, "inventory-root");
		call->AddStringToArray(arr, "inventory-skeleton");
		call->AddStringToArray(arr, "inventory-lib-root");
		call->AddStringToArray(arr, "inventory-lib-owner");
		call->AddStringToArray(arr, "inventory-skel-lib");
		call->AddStringToArray(arr, "initial-outfit");
		call->AddStringToArray(arr, "gestures");
		call->AddStringToArray(arr, "event_categories");
		call->AddStringToArray(arr, "event_notifications");
		call->AddStringToArray(arr, "classified_categories");
		call->AddStringToArray(arr, "buddy-list");
		call->AddStringToArray(arr, "ui-config");
		call->AddStringToArray(arr, "tutorial_setting");
		call->AddStringToArray(arr, "login-flags");
		call->AddStringToArray(arr, "global-textures");

		if(!rpcConnection_->FinishXMLRPCCall(call))
		    return false;

		bool loginresult = false;
		if ( !authentication )
		{
			clientParameters_.sessionID.FromString(call->GetReplyString("session_id"));
			clientParameters_.agentID.FromString(call->GetReplyString("agent_id"));
			clientParameters_.circuitCode = call->GetReplyInt("circuit_code");
			loginresult = true;
		}
		else if ( authentication && callMethod != std::string("login_to_simulator")) 
		{
			// Authentication results 
			clientParameters_.sessionHash = call->GetReplyString("sessionHash");
			clientParameters_.gridUrl = std::string(call->GetReplyString("gridUrl"));
			clientParameters_.avatarStorageUrl = std::string(call->GetReplyString("avatarStorageUrl"));
			loginresult = true;
		}
		else if ( authentication && callMethod == std::string("login_to_simulator"))
		{
			clientParameters_.sessionID.FromString(call->GetReplyString("session_id"));
			clientParameters_.agentID.FromString(call->GetReplyString("agent_id"));
			clientParameters_.circuitCode = call->GetReplyInt("circuit_code");
			loginresult = true;
		}

        ///\ Todo free later, if reply is needed.
		XMLRPC_RequestFree(call->reply, 1);
		return loginresult;
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

using namespace OpenSimProtocol;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OpenSimProtocolModule)
POCO_END_MANIFEST