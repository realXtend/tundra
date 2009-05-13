#ifndef incl_TPIMSession_h
#define incl_TPIMSession_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "TPIMMessage.h"

namespace Communication
{

	// TODO: ScriptObjectPtr doesn't should not be given to constructor...

	class TPSession : public SessionInterface
	{
		friend class TelepathyCommunication;
	public:
		TPSession(ParticipantPtr originator, Foundation::ScriptObjectPtr python_communication_object);
		virtual void Close();
		virtual void SendInvitation(ContactPtr c);
		virtual void Kick(ParticipantPtr p);
		virtual ParticipantListPtr GetParticipants();
		virtual std::string GetProtocol();
		virtual ParticipantPtr GetOriginator();
	protected:
		virtual void NotifyClosedByRemote(); // Called by TelepathyCommunication
		virtual std::string GetId(); // called by TelepathyCommunication
		
		
		std::string id_;
		std::string protocol_;
		Foundation::ScriptObjectPtr python_communication_object_; 
		ParticipantListPtr participants_;
		ParticipantPtr originator_;
	};


	/**
	 * Instant message session. Capable to send IMMessagePtr objects
	 **/ 
	class TPIMSession : public TPSession, public IMSessionInterface
	{
		friend class TelepathyCommunication;
	public:
		TPIMSession(ParticipantPtr originator, Foundation::ScriptObjectPtr python_communication_object);
		virtual void SendIMMessage(IMMessagePtr m);
		virtual IMMessageListPtr GetMessageHistory();

		// /note: These should not be here but because interface heritance (Session, IMSessio) problems we declarate these here too.
		virtual void Close();
		virtual void SendInvitation(ContactPtr c);
		virtual void Kick(ParticipantPtr p);
		virtual ParticipantListPtr GetParticipants();
		virtual std::string GetProtocol();
		virtual ParticipantPtr GetOriginator();



	protected:
		void NotifyMessageReceived(IMMessagePtr m); // called by TelepathyCommunication

		IMMessageList im_messages_;
	};

} // end of namespace: Communication


#endif // incl_TPIMSession_h