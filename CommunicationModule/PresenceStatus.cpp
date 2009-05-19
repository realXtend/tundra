
#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationManager.h"
#include "PresenceStatus.h"


namespace Communication
{
	std::vector<std::string> PresenceStatus::online_status_options_;

	PresenceStatus::PresenceStatus(): online_status_("offline"), online_message_("")
	{

	}

	void PresenceStatus::SetOnlineStatus(std::string status)
	{
		this->online_status_ = status;	
	}

	std::string PresenceStatus::GetOnlineStatus()
	{
		return this->online_status_;
	}

	void PresenceStatus::SetOnlineMessage(std::string message)
	{
		this->online_message_ = message;
	}

	std::string PresenceStatus::GetOnlineMessage()
	{
		return this->online_message_;
	}

	void PresenceStatus::NotifyUpdate(const std::string &online_status, const std::string &online_message)
	{
		online_status_ = online_status;
		online_message_ = online_message;
	}

	std::vector<std::string> PresenceStatus::GetOnlineStatusOptions()
	{
		std::vector<std::string> options;
		for (int i=0; i < online_status_options_.size(); i++)
		{
			options.push_back( online_status_options_[i] );
		}
		return options;
	}

	/**
	 *  @todo send the online_message_ too
	 */
	void PresenceStatus::UpdateToServer()
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CSetStatus", online_status_);
	}

} // end of namespace: Communication