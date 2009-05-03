
#include "StableHeaders.h"
#include "Foundation.h"

#include "TPPresence.h"


namespace Communication
{
	void TPPresenceStatus::SetOnlineStatus(bool status)
	{
		online_status_ = status;
	}

	bool TPPresenceStatus::GetOnlineStatus()
	{
		return online_status_;
	}

	void TPPresenceStatus::SetOnlineMessage(std::string message)
	{
		online_message_ = message;
	}

	std::string TPPresenceStatus::GetOnlineMessage()
	{
		return online_message_;
	}

} // end of namespace: Communication