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
		void SetOnlineStatus(std::string status);
		std::string GetOnlineStatus();
		void SetOnlineMessage(std::string message);
		std::string GetOnlineMessage();
	protected:
		void NotifyUpdate(std::string online_status, std::string online_message);

		std::string id_; // contact id on python side -> do we need it here ?
		std::string online_status_;
		std::string online_message_;
	};
}

#endif // incl_TPPresenceStatus_h