#ifndef incl_CommunicationsEvents_h
#define incl_CommunicationsEvents_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	/*
	*/
	class IMMessageEvent : public Events::IMMessageEventInterface
	{
	public:
		IMMessageEvent(IMSessionPtr s, IMMessagePtr m);
		IMMessagePtr GetMessage();
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
		IMMessagePtr message_;
	};
//	typedef boost::shared_ptr<IMMessageEvent> IMMessageEventPtr;


	/*

	*/
	class PresenceStatusUpdateEvent : public Events::PresenceStatusUpdateEventInterface
	{
	public:
		PresenceStatusUpdateEvent(ContactPtr );
		ContactPtr GetContact();
	private:
		ContactPtr contact_;
	};
	


	/*

	*/
	class IMSessionRequestEvent : public Events::IMSessionRequestEventInterface
	{
	public:
		IMSessionRequestEvent(IMSessionPtr s, ContactPtr c);
		virtual ContactPtr GetContact();
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
		ContactPtr contact_;
	};

	/*

	*/
	class IMSessionEndEvent : public Events::IMSessionEndEventInterface
	{
	public:
		IMSessionEndEvent(IMSessionPtr s);
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
	};
//	typedef boost::shared_ptr<IMSessionEndEvent> IMSessionEndEventPtr;

	class FriendRequestEvent : public Events::FriendRequestEventInterface
	{
	public:
		FriendRequestEvent(FriendRequestPtr r);
		FriendRequestPtr GetFriendRequest();
	private:
		FriendRequestPtr friend_request_;
	};
//	typedef boost::shared_ptr<FriendRequestEvent> FriendRequestEventPtr;


} // end of namespace: Communication

#endif // incl_CommunicationsEvents_h