
#include <Poco/Logger.h>
//#include <Poco/ClassLibrary.h>
#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationManager.h"

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
		LogInfo("CommunicationManager AddListener called");
	}

	void CommunicationManager::RemoveListener(Foundation::CommunicationListener *listener)
	{
		LogInfo("CommunicationManager RemoveListener called");
	}

	void CommunicationManager::Connect()
	{
		LogInfo("CommunicationManager Connect");
	}
	void CommunicationManager::Disconnect()
	{
		LogInfo("CommunicationManager Disconnect");
	}

}