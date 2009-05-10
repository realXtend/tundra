#ifndef incl_TPIMSession_h
#define incl_TPIMSession_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "TPIMMessage.h"

namespace Communication
{


	class TPSession : public SessionInterface
	{
		friend class TelepathyCommunication;
	public:
		TPSession(Foundation::ScriptObjectPtr python_communication_object);
		virtual void Close();
		virtual void SendInvitation(ContactPtr c);
		virtual void Kick(ParticipantPtr p);
		virtual ParticipantListPtr GetParticipants();
		virtual std::string GetProtocol();
	protected:
		virtual void NotifyClosedByRemote(); // Called by TelepathyCommunication
		virtual std::string GetId(); // called by TelepathyCommunication
		
		
		std::string id_;
		std::string protocol_;
		Foundation::ScriptObjectPtr python_communication_object_; 
		ParticipantListPtr participants_;
	};


	// Instant messaging session
	class TPIMSession : public TPSession, public IMSessionInterface
	{
		friend class TelepathyCommunication;
	public:
		TPIMSession(Foundation::ScriptObjectPtr python_communication_object);
		virtual void SendIMMessage(IMMessagePtr m);
		virtual IMMessageListPtr GetMessageHistory();

	protected:
		void NotifyMessageReceived(IMMessagePtr m); // called by TelepathyCommunication

		IMMessageList im_messages_;
	};

} // end of namespace: Communication


#endif // incl_TPIMSession_h