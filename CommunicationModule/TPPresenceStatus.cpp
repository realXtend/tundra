
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

	void TPPresenceStatus::UpdateToServer()
	{
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, online_status_.c_str());
		args[0] = buf1;
		std::string method = "CSetStatus"; 
		std::string syntax = "s";
		Foundation::ScriptObject* ret = TelepathyCommunication::GetInstance()->python_communication_object_->CallMethod(method, syntax, args);
	}

} // end of namespace: Communication