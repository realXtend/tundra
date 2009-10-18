#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
#include "OpensimIM\ConnectionProvider.h"

namespace Communication
{

	CommunicationModule::CommunicationModule(void):ModuleInterfaceImpl("CommunicationModule"), communication_manager_(NULL), console_ui_(NULL), qt_ui_(NULL), communication_service_(NULL)
	{
	}

	CommunicationModule::~CommunicationModule(void)
	{
	}

	void CommunicationModule::Load(){}
	void CommunicationModule::Unload(){}

	void CommunicationModule::Initialize() 
	{
		// current way
		communication_manager_ = TpQt4Communication::CommunicationManager::GetInstance();
//		console_ui_ = new CommunicationUI::ConsoleUI(framework_);
		qt_ui_ = new CommunicationUI::QtGUI(framework_);

		// new way

		CommunicationService::CreateInstance(framework_);
		communication_service_ = CommunicationService::GetInstance();
		OpensimIM::ConnectionProvider* p = new OpensimIM::ConnectionProvider(framework_);
		communication_service_->RegisterConnectionProvider(p);

		LogInfo("Initialized.");
	}

	void CommunicationModule::PostInitialize()
	{
		Foundation::EventManagerPtr event_manager = framework_->GetEventManager(); 

		if ( communication_service_ == NULL)
		{
			return;
		}

		QStringList protocols = communication_service_->GetSupportedProtocols();
		if (protocols.size() == 0)
			LogInfo("No IM protocols supported");
		else
		{
			for (QStringList::iterator i = protocols.begin(); i != protocols.end(); ++i)
			{
				QString message = "IM protocol support for: ";
				message.append(*i);
				LogInfo( message.toStdString() );
			}
		}
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

		LogInfo("Uninitialized.");
	}

	void CommunicationModule::Update(Core::f64 frametime)
	{
	}

    bool CommunicationModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
		if (communication_service_)
			return dynamic_cast<CommunicationService*>( communication_service_ )->HandleEvent(category_id, event_id, data);
        return false;
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
