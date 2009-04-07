
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
	void CommunicationManager::AddListener(Foundation::Comms::CommunicationListener *listener)
	{
		CommunicationModule::LogInfo("CommunicationManager AddListener called");
	}

	void CommunicationManager::RemoveListener(Foundation::Comms::CommunicationListener *listener)
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

	//std::vector<Foundation::Comms::SettingsAttribute> CommunicationManager::GetAccountAttributes()
	//{		
	//	std::vector<Foundation::Comms::SettingsAttribute> settings;
	//	Foundation::Comms::SettingsAttribute name;
	//	name.name = "name";
	//	name.type = Foundation::Comms::CommSettingsType::String;
	//	settings.insert(name);
	//	settings.insert(type);
	//	return settings;
	//}
	//void CommunicationManager::SetAccountAttributes(std::vector<Foundation::Comms::SettingsAttribute> attributes)
	//{
	//	int size = attributes.size();		
	//	//attributes.pop_back()
	//}

	std::map<std::string, Foundation::Comms::SettingsAttribute> CommunicationManager::GetAccountAttributes()
	{
		std::map<std::string, Foundation::Comms::SettingsAttribute> attrs;
		Foundation::Comms::SettingsAttribute name;
		//name.type = Foundation::Comms::CommSettingsType::String;
		name.type = Foundation::Comms::String;
		name.value = "";
		Foundation::Comms::SettingsAttribute password;
		//password.type = Foundation::Comms::CommSettingsType::String;
		password.type = Foundation::Comms::String;
		password.value = "";
		attrs["password"] = password;
		attrs["name"] = name;

		

		//!!
		std::map<std::string, int> things;
		things["this"] = 1;
		things["that"] = 2;
		std::map<std::string, int>::iterator p = things.begin();
		std::cout << p->first << ": " << p->second;

		std::map<std::string, Foundation::Comms::SettingsAttribute>::iterator iter = attrs.find("name");
		std::cout << iter->first;

		//CommunicationModule::LogInfo(attrs.find("name")->);
		return attrs;
	}
	
	

	void CommunicationManager::SetAccountAttributes(std::map<std::string, Foundation::Comms::SettingsAttribute> attributes)	
	{	
		//std::map<std::string, Foundation::Comms::SettingsAttribute>::iterator iter = attributes.find("name");
		account.name = attributes.find("name")->second.value;
		account.password = attributes.find("password")->second.value;
	}


	void CommunicationManager::Log(std::string str)
	{
		CommunicationModule::LogInfo(str);
	}
}