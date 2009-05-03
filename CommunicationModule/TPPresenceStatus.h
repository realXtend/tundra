#ifndef incl_TPPresenceStatus_h
#define incl_TPPresenceStatus_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	class TPPresenceStatus: PresenceStatus
	{
	public:
		void SetOnlineStatus(bool status);
		bool GetOnlineStatus();
		void SetOnlineMessage(std::string message);
		std::string GetOnlineMessage();
	private:
		bool online_status_;
		std::string online_message_;
	};
}

#endif // incl_TPPresenceStatus_h