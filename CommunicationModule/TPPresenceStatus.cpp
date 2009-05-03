
#include "StableHeaders.h"
#include "Foundation.h"

#include "TPPresenceStatus.h"


namespace Communication
{

void TPPresenceStatus::SetOnlineStatus(bool status)
{
	this->online_status_ = status;	
}

bool TPPresenceStatus::GetOnlineStatus()
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

} // end of namespace: Communication