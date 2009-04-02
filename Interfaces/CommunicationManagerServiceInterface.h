#ifndef incl_ICommunicationManagerServiceInterface_h
#define incl_ICommunicationManagerServiceInterface_h

#include "ServiceInterface.h"


namespace Foundation
{
	class CommunicationListener
	{
	public:
		CommunicationListener() {}
		virtual ~CommunicationListener() {}
	};

	class CommunicationManagerServiceInterface : public ServiceInterface
	{
	public:
		CommunicationManagerServiceInterface() {}
		virtual ~CommunicationManagerServiceInterface() {}

		// Communications API	
		// For receiving comm events
		virtual void AddListener(CommunicationListener *listener) = 0;
		virtual void RemoveListener(CommunicationListener *listener) = 0;

		// Init comm events
		virtual void Connect() = 0;
		virtual void Disconnect() = 0;
	};

	typedef boost::shared_ptr<CommunicationManagerServiceInterface> CommunicationManagerPtr;

}

#endif