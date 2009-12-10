#include <StableHeaders.h>
#include "CommunicationModule.h"
#include <RexLogicModule.h>
#include "OpensimIM/ConnectionProvider.h"
#include "TelepathyIM/ConnectionProvider.h"

namespace Communication
{

	CommunicationModule::CommunicationModule(void) 
        : ModuleInterfaceImpl("CommunicationModule"), 
          qt_ui_(0), opensim_ui_(0), 
          master_test_(0), 
          communication_service_(0), 
          test_(0), 
          event_category_networkstate_(0), 
          event_category_framework_(0)
	{
	}

	CommunicationModule::~CommunicationModule(void)
	{
	}

	void CommunicationModule::Load(){}
	void CommunicationModule::Unload(){}

	void CommunicationModule::Initialize() 
	{
        event_category_framework_ = framework_->GetEventManager()->QueryEventCategory("Framework");
		LogInfo("Initialized.");
	}

	void CommunicationModule::PostInitialize()
	{
		Foundation::EventManagerPtr event_manager = framework_->GetEventManager(); 
        CommunicationService::CreateInstance(framework_);
        communication_service_ = CommunicationService::GetInstance();
        if (communication_service_ == 0)
        {
            LogError("Cannot initialize CommunicationService object");
            return;
        }
        
        connect(communication_service_, SIGNAL( NewProtocolSupported(QString &) ), SLOT( OnNewProtocol(QString &) ));
        connect(communication_service_, SIGNAL( ProtocolSupportEnded(QString &) ), SLOT( OnProtocolSupportEnded(QString &) ));
        QObject::connect(communication_service_, SIGNAL( ConnectionOpened(Communication::ConnectionInterface*) ), this, SLOT( OnConnectionOpened(Communication::ConnectionInterface*) ));
        QObject::connect(communication_service_, SIGNAL( ConnectionClosed(Communication::ConnectionInterface*) ), this, SLOT( OnConnectionClosed(Communication::ConnectionInterface*) ));

        OpensimIM::ConnectionProvider* opensim = new OpensimIM::ConnectionProvider(framework_);
        communication_service_->RegisterConnectionProvider(opensim);

	    TelepathyIM::ConnectionProvider* telepathy = new TelepathyIM::ConnectionProvider(framework_);
	    communication_service_->RegisterConnectionProvider(telepathy);

        test_ = new CommunicationTest::Test(framework_);

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
        SAFE_DELETE(master_test_);
		SAFE_DELETE(qt_ui_);
		SAFE_DELETE(opensim_ui_);
        SAFE_DELETE(test_);

        CommunicationService::CleanUp();
        communication_service_ = NULL;

		LogInfo("Uninitialized.");   
	}

	void CommunicationModule::Update(Core::f64 frametime)
	{
	}

    bool CommunicationModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        event_category_networkstate_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");

        if (category_id == event_category_networkstate_)
        {
            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
            {
                boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> current_protocol_module = framework_->GetModuleManager()->GetModule<RexLogic::RexLogicModule>(Foundation::Module::MT_WorldLogic).lock().get()->GetServerConnection()->GetCurrentProtocolModuleWeakPointer();
	    		if (current_protocol_module.lock().get())
                {
                    ProtocolUtilities::ClientParameters client_params = current_protocol_module.lock()->GetClientParameters();
                    //! TODO: Currently we can have only one world_chat ui but this
                    //!       should be changed to 1:1 relation between "WorldChatWidget" - "WorldConnectionWidget"
                    SAFE_DELETE(opensim_ui_);
                    opensim_ui_ = new CommunicationUI::OpenSimChat(framework_, client_params);
                }
            }

            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED || event_id == ProtocolUtilities::Events::EVENT_CONNECTION_FAILED)
            {
               SAFE_DELETE(opensim_ui_);
            }
        }

        if (communication_service_)
            return dynamic_cast<CommunicationService*>( communication_service_ )->HandleEvent(category_id, event_id, data);


		return false;
    }    

    void CommunicationModule::OnNewProtocol(QString &protocol)
    {
        if (protocol.compare("jabber") == 0)
        {
            if (!qt_ui_)
            {
                //master_test_ = new CommunicationUI::MasterWidget(framework_);
                qt_ui_ = new CommunicationUI::QtGUI(framework_);
            }
            return;
        }
    }

    void CommunicationModule::OnProtocolSupportEnded(QString &protocol)
    {
        if (protocol.compare("jabber") == 0)
        {
            SAFE_DELETE(qt_ui_);
            return;
        }
    }

    void CommunicationModule::OnConnectionOpened(Communication::ConnectionInterface* connection)
    {
    }

    void CommunicationModule::OnConnectionClosed(Communication::ConnectionInterface* connection)
    {
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
