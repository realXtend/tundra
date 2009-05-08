#ifndef incl_TPPresenceStatus_h
#define incl_TPPresenceStatus_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	class TPPresenceStatus: PresenceStatus
	{
		friend class TelepathyCommunication;
	public:
		void SetOnlineStatus(bool status);
		bool GetOnlineStatus();
		void SetOnlineMessage(std::string message);
		std::string GetOnlineMessage();
	protected:
		void NotifyUpdate(bool online_status, std::string online_message);

		std::string id_; // presence id on python side
		bool online_status_;
		std::string online_message_;
	};

//	typedef boost::shared_ptr<TPPresenceStatus> TPPresenceStatusPtr;
}

#endif // incl_TPPresenceStatus_h