
#include <Poco/Logger.h>
//#include <Poco/ClassLibrary.h>
#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationManager.h"
#include "CommunicationModule.h"

//POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
//POCO_EXPORT_CLASS(Communication::CommunicationManager)
//POCO_END_MANIFEST


namespace Communication
{

	CommunicationManager::CommunicationManager(void){}
	CommunicationManager::~CommunicationManager(void){}

	// Communications API	
	void CommunicationManager::AddListener(Foundation::CommunicationListener *listener)
	{
		CommunicationModule::LogInfo("CommunicationManager AddListener called");
	}

	void CommunicationManager::RemoveListener(Foundation::CommunicationListener *listener)
	{
		CommunicationModule::LogInfo("CommunicationManager RemoveListener called");
	}

	void CommunicationManager::Connect()
	{
		CommunicationModule::LogInfo("CommunicationManager Connect");
	}
	void CommunicationManager::Disconnect()
	{
		CommunicationModule::LogInfo("CommunicationManager Disconnect");
	}

}