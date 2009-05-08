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

	class TPSession : public Session
	{
		friend class TelepathyCommunication;
	public:
		TPSession(Foundation::ScriptObjectPtr python_communication_object);
		virtual void Close();
		virtual void SendInvitation(ContactPtr c);
		virtual void Kick(Participiant *p);
		virtual ParticipientListPtr GetParticipients();
	protected:
		virtual void NotifyClosedByRemote(); // Called by TelepathyCommunication
		virtual std::string GetId(); // called by TelepathyCommunication
		
		std::string id_;
		Foundation::ScriptObjectPtr python_communication_object_; 
		ParticipientListPtr participients_;
	};


//	class TPIMSession;
//	typedef boost::shared_ptr<TPIMSession> TPIMSessionPtr;
//	typedef std::vector<TPIMSessionPtr> TPIMSessionList;

	// Instant messaging session
	class TPIMSession : public TPSession, public IMSession
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
	//typedef boost::shared_ptr<TPIMSession> TPIMSessionPtr;

} // end of namespace: Communication





// For windows compatibility
#ifdef SendMessageDefined
#define SendMessage SendMessageW
#endif // SendMessageDefined


#endif // incl_TPIMSession_h