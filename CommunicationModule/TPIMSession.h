#ifndef incl_TPIMSession_h
#define incl_TPIMSession_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "TPIMMessage.h"

// For windows compatibility
#ifdef SendMessage
#define SendMessageDefined
#undef SendMessage
#endif // SendMessage

namespace Communication
{

	class TPSession:Session
	{
		friend class TelepathyCommunication;
	public:
		TPSession(Foundation::ScriptObjectPtr python_communication_object);
		virtual int GetId();
		virtual void Close();
		virtual void SendInvitation(ContactPtr c);
		virtual void Kick(Participiant *p);
		virtual ParticipientListPtr GetParticipients();
	protected:
		void NotifyClosedByRemote(); // Called by TelepathyCommunication
		
		int id_;
		Foundation::ScriptObjectPtr python_communication_object_; 

	};

	typedef boost::shared_ptr<TPSession> TPSessionPtr;


	class TPIMSession;
	typedef boost::shared_ptr<TPIMSession> TPIMSessionPtr;
	typedef std::vector<TPIMSessionPtr> TPIMSessionList;

	// Instant messaging session
	class TPIMSession : TPSession
	{
		friend class TelepathyCommunication;
	public:
		TPIMSession(Foundation::ScriptObjectPtr python_communication_object);
		virtual void SendMessage(IMMessagePtr m);
		virtual IMMessageListPtr GetMessageHistory();
	protected:
		void NotifyMessageReceived(TPIMMessagePtr m); // called by TelepathyCommunication
		TPIMMessageList message_history_;
	};
	typedef boost::shared_ptr<TPIMSession> TPIMSessionPtr;

} // end of namespace: Communication





// For windows compatibility
#ifdef SendMessageDefined
#define SendMessage SendMessageW
#endif // SendMessageDefined


#endif // incl_TPIMSession_h