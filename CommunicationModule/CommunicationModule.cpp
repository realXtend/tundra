
//#include <Poco/ClassLibrary.h>
#include <Poco/Logger.h>
//#include <sstream>

#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationModule.h"
#include "CommunicationManager.h"


//namespace Communication
//{

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
		communication_manager_ = Foundation::CommunicationManagerPtr(new CommunicationManager());
		framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_CommunicationManager, communication_manager_.get());
		LogInfo("Module " + Name() + " initialized.");
	}

	void CommunicationModule::PostInitialize(){}

	void CommunicationModule::Uninitialize()
	{
        framework_->GetServiceManager()->UnregisterService(communication_manager_.get());
		communication_manager_.reset();
		LogInfo("Module " + Name() + " uninitialized.");
	}

	void CommunicationModule::Update(){}


	// Communications API
	//void CommunicationModule::AddListener(ICommunicationListener *listener){}
	//void CommunicationModule::RemoveListener(ICommunicationListener *listener){}
	//void CommunicationModule::Connect(){}
	//void CommunicationModule::Disconnect(){}
//}

//using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationModule)
POCO_END_MANIFEST
