
#include <Poco/Logger.h>

#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
#include "CommunicationManager.h"


namespace Communication
{

	CommunicationModule::CommunicationModule(void):ModuleInterfaceImpl("CommunicationModule")
	{
	}

	CommunicationModule::~CommunicationModule(void)
	{
	}


	void CommunicationModule::Load(){}
	void CommunicationModule::Unload(){}

	void CommunicationModule::Initialize() 
	{
		communication_manager_ = Foundation::Comms::CommunicationManagerPtr(new CommunicationManager());
		framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_CommunicationManager, communication_manager_.get());
		LogInfo("Module " + Name() + " initialized.");


		communication_manager = CommunicationServicePtr(new TelepathyCommunicationManager());
		framework_->GetServiceManager();

		//testing using py service from here, 'cause had trouble with loading in the unit test
		/* commented to not bork for ppl without py
		Foundation::ScriptServiceInterface *pyengine = framework_->GetService<Foundation::ScriptServiceInterface>
			(Foundation::Service::ST_Scripting);
		pyengine->RunString("print 'Hello Python from CommunicationModule!'");
		pyengine->RunScript("communication"); *//* loads the module, 
		i.e. brings comms module to the global namespace
		so that the contents of it can be referred to as comms.x */
	}

	void CommunicationModule::PostInitialize(){}

	void CommunicationModule::Uninitialize()
	{
        framework_->GetServiceManager()->UnregisterService(communication_manager_.get());
		communication_manager_.reset();
		LogInfo("Module " + Name() + " uninitialized.");
	}

	void CommunicationModule::Update(Core::f64 frametime)
	{
		//if done this way, should keep the ref that got in init
		/*Foundation::ScriptServiceInterface *pyengine = framework_->GetService<Foundation::ScriptServiceInterface>
			(Foundation::Service::ST_Scripting);

		pyengine->RunString("communication.update()"); //XXX no way to get return val, w.i.p*/
	}


	// Communications API
	//void CommunicationModule::AddListener(ICommunicationListener *listener){}
	//void CommunicationModule::RemoveListener(ICommunicationListener *listener){}
	//void CommunicationModule::Connect(){}
	//void CommunicationModule::Disconnect(){}
}

using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationModule)
POCO_END_MANIFEST
