#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
//#include "CommunicationManager.h"


namespace Communication
{

	CommunicationModule::CommunicationModule(void):ModuleInterfaceImpl("CommunicationModule"), communication_manager_(NULL), console_ui_(NULL)
	{
	}

	CommunicationModule::~CommunicationModule(void)
	{
	}


	void CommunicationModule::Load(){}
	void CommunicationModule::Unload(){}

	void CommunicationModule::Initialize() 
	{
		
		communication_manager_ = TpQt4Communication::CommunicationManager::GetInstance();
		console_ui_ = new CommunicationUI::ConsoleUI(framework_);
		qt_ui_ = new CommunicationUI::QtUI(0, framework_);
		qt_ui_->show();

		if (communication_manager_->GetState() == TpQt4Communication::CommunicationManager::STATE_ERROR)
		{
			LogError("Initialization failed.");
			return;
		}
//		c.SetProtocol("jabber");
//		c.SetUserID("kuonanoja@jabber.org");
//		c.SetPassword("jabber666");
		//comm_->OpenConnection();
		//communication_manager_ = CommunicationManagerPtr(new CommunicationManager(framework_));
		//if (communication_manager_->IsInitialized())
//		    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Communication, communication_manager_ );
		LogInfo("Initialized.");
	}

	void CommunicationModule::PostInitialize()
	{
	}

	void CommunicationModule::Uninitialize()
	{
		if (console_ui_)
		{
			delete console_ui_;
			console_ui_ = NULL;
		}

		if (qt_ui_)
		{
			delete qt_ui_;
			qt_ui_ = NULL;
		}

		if (communication_manager_)
		{
			delete communication_manager_;
			communication_manager_ = NULL;
		}

	 //   if (communication_manager_ && communication_manager_->IsInitialized())
		//{
  //          framework_->GetServiceManager()->UnregisterService(communication_manager_);
		//	communication_manager_->UnInitialize();
		//}
		// TODO: Uninitialize communication manager
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
