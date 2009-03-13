// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimProtocolModule.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"

#include "md5wrapper.h"
#include "PocoXMLRPC.h"
#include "OpenSimAuth.h"

namespace OpenSimProtocol
{
	OpenSimProtocolModule::OpenSimProtocolModule() : ModuleInterface_Impl(Foundation::Module::MT_Network)
    {
    }

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
    void OpenSimProtocolModule::Initialize(Foundation::Framework *framework)
    {
        assert(framework != NULL);
        framework_ = framework;
        
		const char *filename = "./data/message_template.msg";
		//const std::string &filename = OpenSimProtocolModule::GetTemplateFilename();
		networkManager_ = shared_ptr<NetMessageManager>(new NetMessageManager(filename));
		
        LogInfo("System " + Name() + " initialized.");
    }

    // virtual 
    void OpenSimProtocolModule::Uninitialize(Foundation::Framework *framework)
    {
        assert(framework_ != NULL);
        framework_ = NULL;
		
		//networkManager_->Disconnect();
        
		LogInfo("System " + Name() + " uninitialized.");
    }

    // virtual
    void OpenSimProtocolModule::Update()
    {
        /*LogInfo("Updating " + Name());
		
		// create new entity
        LOG("Constructing entity with component: " + Test::EC_Dummy::Name() + ".");
		
        Foundation::EntityPtr entity = framework_->GetEntityManager()->createEntity();
        assert (entity.get() != 0 && "Failed to create entity.");

        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent(Test::EC_Dummy::Name());
        assert (component.get() != 0 && "Failed to create dummy component.");

        entity->addEntityComponent(component);
        component = entity->getComponent(component->_Name());
        assert (component.get() != 0 && "Failed to get dummy component from entity.");

        Foundation::TestServiceInterface *test_service = framework_->GetServiceManager()->GetService<Foundation::TestServiceInterface>(Foundation::Service::ST_Test);
        assert (test_service != NULL);
        assert (test_service->Test());

        framework_->Exit();
        assert (framework_->IsExiting());*/
    }

	/*const std::string &OpenSimProtocolModule::GetTemplateFilename()
	{
        try
        {
			config_ = new Poco::Util::XMLConfiguration("./bin/modules/core/OpenSimProtocolModule.xml");
        } catch (std::exception &e)
        {
            // not fatal
            LogError(e.what());
            LogError("Failed to load message template filename.");
        }

		//if (!configuration_.isNull())
			const std::string &filename = config_->getString("message_template");
		
		return filename;
	}*/
	
	void OpenSimProtocolModule::AddListener(INetMessageListener *listener)
	{
	    networkManager_->SetNetworkListener(listener);
	}
	
	///\todo ?
	/*void OpenSimProtocolModule::Removeregister(INetMessageListener *listener)
	{
	    //networkManager_->
	}*/
	
	void OpenSimProtocolModule::ConnectToRexServer(
	    const char *first_name,
		const char *last_name,
		const char *password,
		const char *address,
		int port)
	{
	    PerformXMLRPCLogin(first_name, last_name, password, address, port);
	    networkManager_->ConnectTo(address, port);
	}
	
    void OpenSimProtocolModule::PerformXMLRPCLogin(
        const char *first_name,
        const char *last_name,
        const char *password,
        const char *address,
        int port)
    {
		// create a MD5 hash for the password, MAC address and HDD serial number.
		std::string mac_addr = GetMACaddressString();
		std::string id0 = GetId0String();

		md5wrapper md5;
		std::string password_hash = md5.getHashFromString(password);
		std::string mac_hash = md5.getHashFromString(mac_addr);
		std::string id0_hash = md5.getHashFromString(id0);

		rpcConnection_ = shared_ptr<PocoXMLRPCConnection>(new PocoXMLRPCConnection(address, port));

		boost::shared_ptr<PocoXMLRPCCall> call = rpcConnection_->StartXMLRPCCall("login_to_simulator");

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

		rpcConnection_->FinishXMLRPCCall(call);

		sessionID = call->GetReplyString("session_id");
		mySessionID.FromString(sessionID);
		agentID = call->GetReplyString("agent_id");
		myAgentID.FromString(agentID);
		circuitCode = call->GetReplyInt("circuit_code");
	}
	
}

using namespace OpenSimProtocol;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OpenSimProtocolModule)
POCO_END_MANIFEST