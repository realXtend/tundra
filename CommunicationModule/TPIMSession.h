#ifndef incl_TPIMSession_h
#define incl_TPIMSession_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "TPIMMessage.h"

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
	protected:
		void NotifyMessageReceived(TPIMMessagePtr m); // called by TelepathyCommunication
		TPIMMessageList message_history_;
	};

} // end of namespace: Communication

#endif // incl_TPIMSession_h
