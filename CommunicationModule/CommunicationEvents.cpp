
#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationEvents.h"


namespace Communication
{
	IMMessageEvent::IMMessageEvent(IMSessionPtr s, IMMessagePtr m): session_(s), message_(m)
	{

	}

	IMMessagePtr IMMessageEvent::GetMessage()
	{
		return message_;
	}

	IMSessionPtr IMMessageEvent::GetSession()
	{
		return session_;
	}

	PresenceStatusUpdateEvent::PresenceStatusUpdateEvent(ContactPtr c)
	{
		contact_ = c;
	}

	ContactPtr PresenceStatusUpdateEvent::GetContact()
	{
		return contact_;
	}

	IMSessionRequestEvent::IMSessionRequestEvent(IMSessionPtr s, ContactPtr c)
	{
		contact_ = c;
		session_ = s;
	}

	IMSessionPtr IMSessionRequestEvent::GetSession()
	{
		return session_;
	}
	
	ContactPtr IMSessionRequestEvent::GetContact()
	{
		return contact_;
	}

	IMSessionEndEvent::IMSessionEndEvent(IMSessionPtr s)
	{
		session_ = s;
	}

	IMSessionPtr IMSessionEndEvent::GetSession()
	{
		return session_;
	}

	FriendRequestEvent::FriendRequestEvent(FriendRequestPtr r): friend_request_(r)
	{

	}

	FriendRequestPtr FriendRequestEvent::GetFriendRequest()
	{
		return friend_request_;
	}

	SessionStateEvent::SessionStateEvent(int type): type_(type)
	{

	}

	int SessionStateEvent::GetType()
	{
		return type_;
	}

	ConnectionStateEvent::ConnectionStateEvent(int type): type_(type)
	{

	}

	int ConnectionStateEvent::GetType()
	{
		return type_;
	}

} // end of namespace: Communication
