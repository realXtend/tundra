#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
#include "OpensimIM\ConnectionProvider.h"
#include "TelepathyIM\ConnectionProvider.h"

namespace Communication
{

	CommunicationModule::CommunicationModule(void):ModuleInterfaceImpl("CommunicationModule"), communication_manager_(0), console_ui_(0), qt_ui_(0), communication_service_(0), test_(0)
	{
	}

	CommunicationModule::~CommunicationModule(void)
	{
	}

	void CommunicationModule::Load(){}
	void CommunicationModule::Unload(){}

	void CommunicationModule::Initialize() 
	{
		// new way

		CommunicationService::CreateInstance(framework_);
		communication_service_ = CommunicationService::GetInstance();

		OpensimIM::ConnectionProvider* opensim = new OpensimIM::ConnectionProvider(framework_);
		communication_service_->RegisterConnectionProvider(opensim);

		//! @note DO NOT use the old and the new way at the same time.
		//!       DBus service cannot have two instanced.
		TelepathyIM::ConnectionProvider* telepathy = new TelepathyIM::ConnectionProvider(framework_);
		communication_service_->RegisterConnectionProvider(telepathy);

		test_ = new CommunicationTest::Test();

		// current way

//		communication_manager_ = TpQt4Communication::CommunicationManager::GetInstance();
//		console_ui_ = new CommunicationUI::ConsoleUI(framework_);
//		qt_ui_ = new CommunicationUI::QtGUI(framework_);

		boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
        if (console_service)
        {
			console_service->RegisterCommand(Console::CreateCommand("comm test", "Run a test for communication service", Console::Bind(this, &CommunicationModule::Test)));
		}

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

	Console::CommandResult CommunicationModule::Test(const Core::StringVector &params)
	{
		if (params.size() != 1)
		{
			return Console::ResultFailure("Wrong sumner of arguments!\nUse: comm test(T) where T is the id of the test.");
		}
		QString test_id = QString(params[0].c_str());
		switch( test_id.toInt() )
		{
			case 1: test_->RunTest1();break;
			case 2: test_->RunTest2();break;
		}
		return Console::ResultSuccess("");
	}

	void CommunicationModule::Uninitialize()
	{
		if (console_ui_)
			SAFE_DELETE(console_ui_)

		if (qt_ui_)
			SAFE_DELETE(qt_ui_);

		if (communication_manager_)
			SAFE_DELETE(communication_manager_);

		if (communication_service_)
			SAFE_DELETE(communication_service_);

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
