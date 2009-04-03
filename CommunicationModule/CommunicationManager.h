#ifndef incl_CommunicationManager_h
#define incl_CommunicationManager_h


#include "CommunicationManagerServiceInterface.h"
//#pragma once

namespace Communication
{

	/* Basicly will be a wrapper for underlying (telepathy) communications manager */
													
	class CommunicationManager : public Foundation::CommunicationManagerServiceInterface
	{
	public:
		CommunicationManager(void);
		virtual ~CommunicationManager(void);

		//MODULE_LOGGING_FUNCTIONS

		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Communication;


		// Communications API	
		// For receiving comm events
		virtual void AddListener(Foundation::CommunicationListener *listener);
		virtual void RemoveListener(Foundation::CommunicationListener *listener);

		//virtual void SetLoginAndPassword(std::string login, std::string password);

		// Init comm events
		virtual void Connect();
		virtual void Disconnect();


    private:
        

		
	};

}

#endif