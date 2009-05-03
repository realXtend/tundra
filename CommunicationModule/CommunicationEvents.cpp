
#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationEvents.h"


namespace Communication
{
	IMMessageReceivedEvent::IMMessageReceivedEvent(IMMessagePtr m): message_(m)
	{

	}

	IMMessagePtr IMMessageReceivedEvent::GetMessage()
	{
		return message_;
	}

	PresenceStatusUpdateEvent::PresenceStatusUpdateEvent(PresenceStatusPtr s)
	{
		status_ = s;
	}

	PresenceStatusPtr PresenceStatusUpdateEvent::GetPresenceStatus()
	{
		return status_;
	}

	IMSessionInvitationEvent::IMSessionInvitationEvent(IMSessionPtr s)
	{
		session_ = s;
	}

	IMSessionPtr IMSessionInvitationEvent::GetSession()
	{
		return session_;
	}

	IMSessionClosedEvent::IMSessionClosedEvent(IMSessionPtr s)
	{
		session_ = s;
	}

	IMSessionPtr IMSessionClosedEvent::GetSession()
	{
		return session_;
	}


} // end of namespace: Communication
