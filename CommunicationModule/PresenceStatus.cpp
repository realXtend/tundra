
#include "StableHeaders.h"
#include "Foundation.h"
#include "PresenceStatus.h"


namespace TpQt4Communication
{
	//std::vector<std::string> PresenceStatus::online_status_options_;

	PresenceStatus::PresenceStatus(): status_text_(""), message_text_("")
	{

	}

	void PresenceStatus::SetStatusText(std::string text)
	{
		status_text_ = text;
	}

	std::string PresenceStatus::GetStatusText()
	{
		return status_text_;
	}

	void PresenceStatus::SetMessageText(std::string text)
	{
		message_text_ = text;
	}

	std::string PresenceStatus::GetMessageText()
	{
		return message_text_;
	}


} // end of namespace: Communication