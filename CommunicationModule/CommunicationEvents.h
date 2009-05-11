#ifndef incl_CommunicationsEvents_h
#define incl_CommunicationsEvents_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	/*
	*/
	class IMMessageEvent : public Foundation::EventDataInterface
	{
	public:
		IMMessageEvent(IMSessionPtr s, IMMessagePtr m);
		IMMessagePtr GetMessage();
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
		IMMessagePtr message_;
	};
	typedef boost::shared_ptr<IMMessageEvent> IMMessageEventPtr;


	/*

	*/
	class PresenceStatusUpdateEvent : public Foundation::EventDataInterface
	{
	public:
		PresenceStatusUpdateEvent(ContactPtr );
		ContactPtr GetContact();
	private:
		ContactPtr contact_;
	};
	typedef boost::shared_ptr<PresenceStatusUpdateEvent> PresenceStatusUpdateEventPtr;

	/*

	*/
	
	class IMSessionRequestEvent : public Foundation::EventDataInterface
	{
	public:
		IMSessionRequestEvent(IMSessionPtr s);
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
	};
	typedef boost::shared_ptr<IMSessionRequestEvent> IMSessionRequestEventPtr;

	/*

	*/
	class IMSessionEndEvent : public Foundation::EventDataInterface
	{
	public:
		IMSessionEndEvent(IMSessionPtr s);
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
	};
	typedef boost::shared_ptr<IMSessionEndEvent> IMSessionEndEventPtr;

	class FriendRequestEvent : public Foundation::EventDataInterface
	{
	public:
		FriendRequestEvent(FriendRequestPtr r);
		FriendRequestPtr GetFriendRequest();
	private:
		FriendRequestPtr friend_request_;
	};
	typedef boost::shared_ptr<FriendRequestEvent> FriendRequestEventPtr;


} // end of namespace: Communication

#endif // incl_CommunicationsEvents_h