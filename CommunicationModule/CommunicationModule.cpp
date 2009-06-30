
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
		communication_manager_ = CommunicationManagerPtr(new CommunicationManager(framework_));
		if (communication_manager_->IsInitialized())
		    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Communication, communication_manager_ );
		LogInfo("Initialized.");
	}

	void CommunicationModule::PostInitialize()
	{
	}

	void CommunicationModule::Uninitialize()
	{
	    if (communication_manager_ && communication_manager_->IsInitialized())
		{
            framework_->GetServiceManager()->UnregisterService(communication_manager_);
			communication_manager_->UnInitialize();
		}

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

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationModule)
POCO_END_MANIFEST
