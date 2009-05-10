#ifndef incl_CommunicationsEvents_h
#define incl_CommunicationsEvents_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
    namespace Events
    {
        static const Core::event_id_t PRESENCE_STATUS_UPDATE = 1;
        static const Core::event_id_t IM_MESSAGE = 2;
        static const Core::event_id_t IM_SESSION_REQUEST = 3;
		static const Core::event_id_t IM_SESSION_END = 4;
		static const Core::event_id_t FRIEND_REQUEST = 5;
		static const Core::event_id_t FRIEND_RESPONSE = 6;

		// future events ?
//      static const Core::event_id_t IM_SESSION_STATE_CHANGED = 4;
//		static const Core::event_id_t SESSION_INVITATION_RESPONSE_RECEIVED = 5;
//      static const Core::event_id_t PARTICIPIENT_LEFT = 6;
//		static const Core::event_id_t PARTICIPIENT_JOINED = 7;
//      static const Core::event_id_t FRIENDSHIP_RESPONSE_RECEIVED = 9;
//      static const Core::event_id_t CONNECTION_STATUS_CHANGED = 10;
//      static const Core::event_id_t SESSION_JOIN_REQUEST_RECEIVED = 11;
	}

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