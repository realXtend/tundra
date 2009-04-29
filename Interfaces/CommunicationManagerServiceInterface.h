#ifndef incl_ICommunicationManagerServiceInterface_h
#define incl_ICommunicationManagerServiceInterface_h

#include "ServiceInterface.h"


namespace Foundation
{
	namespace Comms
	{
		class CommunicationListener
		{
		public:
			CommunicationListener() {}
			virtual ~CommunicationListener() {}
		};

		enum CommSettingsType 
		{
			String = 0,
			Int,
			Boolean
		};

		struct SettingsAttribute
		{
			//std::string name;
			CommSettingsType type;
			std::string value;
			int length;
		};


        //! \todo document -cm
        /*!
            \ingroup Services_group
        */
		class CommunicationManagerServiceInterface : public ServiceInterface
		{
		public:

			CommunicationManagerServiceInterface() {}
			virtual ~CommunicationManagerServiceInterface() {}

			// Communications API	
			// For receiving comm events
			virtual void AddListener(CommunicationListener *listener) = 0;
			virtual void RemoveListener(CommunicationListener *listener) = 0;

			// Protocol abstraction layer
			//virtual std::vector<SettingsAttribute> GetAccountAttributes() = 0;
			//virtual void SetAccountAttributes(std::vector<SettingsAttribute> attributes) = 0;
			virtual std::map<std::string, SettingsAttribute> GetAccountAttributes() = 0;
			virtual void SetAccountAttributes(std::map<std::string, SettingsAttribute> attributes) = 0;

			//virtual std::vector<std::string> GetSettingAttributes() = 0;
			//virtual void SetSettingsAttributes(std::vector<std::string> attr) = 0;



			// Init comm events
			virtual void Connect() = 0;
			virtual void Disconnect() = 0;
		};

		typedef boost::shared_ptr<CommunicationManagerServiceInterface> CommunicationManagerPtr;
	}
}

#endif