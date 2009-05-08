#ifndef incl_CommunicationsEvents_h
#define incl_CommunicationsEvents_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
    namespace Events
    {
        static const Core::event_id_t PRESENCE_STATUS_UPDATED = 1;
        static const Core::event_id_t IM_MESSAGE_RECEIVED = 2;
        static const Core::event_id_t IM_SESSION_INVITATION_RECEIVED = 3;
		static const Core::event_id_t SESSION_CLOSED = 4;

		// future events
//        static const Core::event_id_t IM_SESSION_STATE_CHANGED = 4;
//		static const Core::event_id_t SESSION_INVITATION_RESPONSE_RECEIVED = 5;
//        static const Core::event_id_t PARTICIPIENT_LEFT = 6;
//		static const Core::event_id_t PARTICIPIENT_JOINED = 7;
//		static const Core::event_id_t FRIENDSHIP_REQUEST_RECEIVED = 8;
//        static const Core::event_id_t FRIENDSHIP_RESPONSE_RECEIVED = 9;
//        static const Core::event_id_t CONNECTION_STATUS_CHANGED = 10;
		//SESSION_JOIN_REQUEST_RECEIVED
	}

	/*

	*/
	class IMMessageReceivedEvent : public Foundation::EventDataInterface
	{
	public:
		IMMessageReceivedEvent(IMMessagePtr m);
		IMMessagePtr GetMessage();
	private:
		IMMessagePtr message_;
	};

	typedef boost::shared_ptr<IMMessageReceivedEvent> IMMessageReceivedPtr;


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

	/*

	*/
	typedef boost::shared_ptr<PresenceStatusUpdateEvent> PresenceStatusUpdateEventPtr;

	class IMSessionInvitationEvent : public Foundation::EventDataInterface
	{
	public:
		IMSessionInvitationEvent(IMSessionPtr s);
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
	};

	/*

	*/
	class IMSessionClosedEvent : public Foundation::EventDataInterface
	{
	public:
		IMSessionClosedEvent(IMSessionPtr s);
		IMSessionPtr GetSession();
	private:
		IMSessionPtr session_;
	};

} // end of namespace: Communication

#endif // incl_CommunicationsEvents_h