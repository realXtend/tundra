
#include "StableHeaders.h"
#include "Foundation.h"
#include "TelepathyCommunication.h"
#include "TPPresenceStatus.h"


namespace Communication
{
	TPPresenceStatus::TPPresenceStatus()
	{
		online_status_options_ = TelepathyCommunication::GetInstance()->presence_status_options_;
	}

	void TPPresenceStatus::SetOnlineStatus(std::string status)
	{
		this->online_status_ = status;	
	}

	std::string TPPresenceStatus::GetOnlineStatus()
	{
		return this->online_status_;
	}

	void TPPresenceStatus::SetOnlineMessage(std::string message)
	{
		this->online_message_ = message;
	}

	std::string TPPresenceStatus::GetOnlineMessage()
	{
		return this->online_message_;
	}

	void TPPresenceStatus::NotifyUpdate(std::string online_status, std::string online_message)
	{
		online_status_ = online_status;
		online_message_ = online_message;
	}

	std::vector<std::string> TPPresenceStatus::GetOnlineStatusOptions()
	{
		std::vector<std::string> options;
		for (int i=0; i < online_status_options_.size(); i++)
		{
			options.push_back( online_status_options_[i] );
		}
		return options;
	}

} // end of namespace: Communication