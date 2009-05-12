#ifndef incl_CommunicationsEvents_h
#define incl_CommunicationsEvents_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	/*
	 *
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


	/*
	 *
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
	 *
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

	class FriendRequestEvent : public Events::FriendRequestEventInterface
	{
	public:
		FriendRequestEvent(FriendRequestPtr r);
		FriendRequestPtr GetFriendRequest();
	private:
		FriendRequestPtr friend_request_;
	};

	/*
	 *
	 */
	class SessionStateEvent : public  Events::SessionStateEventInterface
	{
	public:
		SessionStateEvent(IMSessionPtr s, int type);
		virtual int GetType();
		virtual IMSessionPtr GetIMSession();
	private:
		static const int UNKNOW = 0;
		int type_;
		IMSessionPtr session_;
	};

	/*
	 *
	 */
	class ConnectionStateEvent: public Events::ConnectionStateEventInterface
	{
	public:
		ConnectionStateEvent(int type);
		virtual int GetType();
	private:
		static const int UNKNOW = 0;
		int type_;
	};


} // end of namespace: Communication

#endif // incl_CommunicationsEvents_h