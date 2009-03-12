// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimProtocolModule.h"
#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"


namespace OpenSimProtocol
{
	OpenSimProtocolModule::OpenSimProtocolModule() : ModuleInterface_Impl(Foundation::Module::Type_Network)
    {
    }

    OpenSimProtocolModule::~OpenSimProtocolModule()
    {
    }

    // virtual
    void OpenSimProtocolModule::Load()
    {
//        using namespace OpenSimProtocol;

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
		
		networkManager_->ConnectTo("192.168.1.144", 9000);
		
        LogInfo("System " + Name() + " initialized.");
    }

    // virtual 
    void OpenSimProtocolModule::Uninitialize(Foundation::Framework *framework)
    {
        //framework_->GetServiceManager()->UnregisterService(&test_service_);

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
	
	/*void OpenSimProtocolModule::AddListener(INetMessageListener *listener)
	{
	    networkManager_->SetNetworkListener(listener);
	}*/
}

using namespace OpenSimProtocol;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(OpenSimProtocolModule)
POCO_END_MANIFEST