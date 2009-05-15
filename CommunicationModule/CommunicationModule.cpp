
#include <Poco/Logger.h>

#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
#include "CommunicationManager.h"
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
		communication_manager_ = CommunicationServicePtr((CommunicationServiceInterface*)new CommunicationManager(framework_));
		LogInfo("Initialized.");
	}

	void CommunicationModule::PostInitialize()
	{
	}

	void CommunicationModule::Uninitialize()
	{
        framework_->GetServiceManager()->UnregisterService(communication_manager_);
		LogInfo("Uninitialized.");
	}

	void CommunicationModule::Update(Core::f64 frametime)
	{
		//if done this way, should keep the ref that got in init
		/*Foundation::ScriptServiceInterface *pyengine = framework_->GetService<Foundation::ScriptServiceInterface>
			(Foundation::Service::ST_Scripting);

		pyengine->RunString("communication.update()"); //XXX no way to get return val, w.i.p*/
	}
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Framework *framework);
void SetProfiler(Foundation::Framework *framework)
{
    Foundation::ProfilerSection::SetProfiler(&framework->GetProfiler());
}

using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationModule)
POCO_END_MANIFEST
