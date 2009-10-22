#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
#include "OpensimIM\ConnectionProvider.h"
#include "TelepathyIM\ConnectionProvider.h"

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
		// new way

		CommunicationService::CreateInstance(framework_);
		communication_service_ = CommunicationService::GetInstance();

		OpensimIM::ConnectionProvider* opensim = new OpensimIM::ConnectionProvider(framework_);
		communication_service_->RegisterConnectionProvider(opensim);

		//! @note DO NOT use the old and the new way at the same time.
		//!       DBus service cannot have two instanced.
		TelepathyIM::ConnectionProvider* telepathy = new TelepathyIM::ConnectionProvider(framework_);
		communication_service_->RegisterConnectionProvider(telepathy);

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
		// Test Telepathy IM
		// Login to given jabber server
		// Fetch contact list 
		// Send a text message to first contact on the list
		try
		{
			CommunicationServiceInterface* communication_service = CommunicationService::GetInstance();
			Credentials jabber_credentials;
			jabber_credentials.SetProtocol("jabber");
			jabber_credentials.SetUserID("rex_user_1@jabber.org");
			jabber_credentials.SetServer("jabber.org");
			jabber_credentials.SetPort(5222);
			jabber_credentials.SetPassword("");
			ConnectionInterface* jabber_connection = communication_service->OpenConnection(jabber_credentials);
			while (jabber_connection->GetState() == Communication::ConnectionInterface::STATE_INITIALIZING)
			{
				QTime wait_time = QTime::currentTime().addSecs(0.100);
				while( QTime::currentTime() < wait_time )
					QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
			}
			Communication::ContactGroupInterface& friend_list = jabber_connection->GetContacts();
			Communication::ContactVector contacts = friend_list.GetContacts();
			for (Communication::ContactVector::iterator i = contacts.begin(); i != contacts.end(); ++i)
			{
				QString name = (*i)->GetName();
				QString message = QString("Friend: ").append(name);
				LogInfo(message.toStdString());
			}
			if (contacts.size() > 0)
			{
				ChatSessionInterface* chat = jabber_connection->OpenPrivateChatSession(*(contacts[0]));
				chat->SendMessage("Hello world!");
				chat->Close();
			}
			jabber_connection->Close();
		}
		catch(Core::Exception &e)
		{
			QString message = QString("Test for TelepathyIM failed: ").append(e.what());
			LogDebug(message.toStdString());
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
