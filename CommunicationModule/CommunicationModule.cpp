
#include <Poco/Logger.h>

#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
#include "CommunicationManager.h"
#include "TelepathyCommunication.h"


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
		// OLD WAY (Still used)
		communication_manager_old_ = Foundation::Comms::CommunicationManagerPtr(new CommunicationManager());
		framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_CommunicationManager, communication_manager_old_.get());
		LogInfo("Module " + Name() + " initialized.");

		// NEW
//		communication_manager_ = CommunicationServicePtr((CommunicationServiceInterface*)new TelepathyCommunication(framework_));
//		framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Communication, communication_manager_.get());
//		LogInfo("Initialized.");
	}

	void CommunicationModule::PostInitialize()
	{
		// todo: Connect with credential from conf file ?
	}

	void CommunicationModule::Uninitialize()
	{
        framework_->GetServiceManager()->UnregisterService(communication_manager_old_.get());
		communication_manager_old_.reset();
		LogInfo("Module " + Name() + " uninitialized.");
	}

	void CommunicationModule::Update(Core::f64 frametime)
	{
		//if done this way, should keep the ref that got in init
		/*Foundation::ScriptServiceInterface *pyengine = framework_->GetService<Foundation::ScriptServiceInterface>
			(Foundation::Service::ST_Scripting);

		pyengine->RunString("communication.update()"); //XXX no way to get return val, w.i.p*/
	}
}

using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationModule)
POCO_END_MANIFEST
