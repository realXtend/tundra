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
        NetworkEventInboundData data(msgID, msg);
        eventManager_->SendEvent(networkEventInCategory_, EVENT_NETWORK_IN, &data);
    }
        
    //virtual
    void OpenSimProtocolModule::OnNetworkMessageSent(const NetOutMessage *msg)
    {
        // Send a NetworkOutStats event.
        NetworkEventOutboundData data(msg->GetMessageID(), msg);
        eventManager_->SendEvent(networkEventOutCategory_, EVENT_NETWORK_OUT, &data);    
    }
    
    bool OpenSimProtocolModule::ConnectToRexServer(
	    const char *first_name,
		const char *last_name,
		const char *password,
		const char *address,
		int port)
	{
	    if (!PerformXMLRPCLogin(first_name, last_name, password, address, port, &clientParameters_))
	        return false;
        if (!networkManager_->ConnectTo(address, port))
            return false;
        bConnected_ = true;
        return true;
	}
	
	void OpenSimProtocolModule::DisconnectFromRexServer()
	{
	    networkManager_->Disconnect();
	    bConnected_ = false;
	}
	
    bool OpenSimProtocolModule::PerformXMLRPCLogin(
        const char *first_name,
        const char *last_name,
        const char *password,
        const char *address,
        int port,
        ClientParameters *params)
    {
		// create a MD5 hash for the password, MAC address and HDD serial number.
		std::string mac_addr = GetMACaddressString();
		std::string id0 = GetId0String();

		md5wrapper md5;
		std::string password_hash = md5.getHashFromString(password);
		std::string mac_hash = md5.getHashFromString(mac_addr);
		std::string id0_hash = md5.getHashFromString(id0);

        try
        {
            rpcConnection_ = shared_ptr<PocoXMLRPCConnection>(new PocoXMLRPCConnection(address, port));
        } catch(std::exception &e)
        {
            LogError("Could not connect to server. Reason: " + Core::ToString(e.what()) + ".");
            return false;
        }

		boost::shared_ptr<PocoXMLRPCCall> call = rpcConnection_->StartXMLRPCCall("login_to_simulator");
        if(!call)
            return false;
            
		call->AddStringMember("first", first_name);
		call->AddStringMember("last", last_name);
		call->AddStringMember("passwd", password_hash.c_str());
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

		params->sessionID.FromString(call->GetReplyString("session_id"));
		params->agentID.FromString(call->GetReplyString("agent_id"));
		params->circuitCode = call->GetReplyInt("circuit_code");
        
        ///\ Todo free later, if reply is needed.
		XMLRPC_RequestFree(call->reply, 1);

		return true;
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