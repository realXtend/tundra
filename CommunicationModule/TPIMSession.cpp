
#include "StableHeaders.h"
#include "Foundation.h"

#include "TPIMSession.h"

// For windows compatibility
#ifdef SendMessage
#define SendMessageDefined
#undef SendMessage
#endif // SendMessage

namespace Communication
{
	TPSession::TPSession(Foundation::ScriptObjectPtr python_communication_object)
	{
		python_communication_object_ = python_communication_object;		
	}

	int TPSession::GetId()
	{
		return id_;
	}

	void TPSession::Close()
	{
		std::string method = "CCloseChannel";
		std::string syntax = "";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
		// todo: inform TelepathyCommunication about end of session
	}

	void TPSession::NotifyClosedByRemote()
	{
	}


	void TPSession::SendInvitation(ContactPtr c)
	{
		// Not implemented in python yet: Multiuser chat
	}

	void TPSession::Kick(Participiant *p)
	{
		// Not implemented in python yet: Multiuser chat
	}

	TPIMSession::TPIMSession(Foundation::ScriptObjectPtr python_communication_object):TPSession(python_communication_object)
	{
	}

	void TPIMSession::SendMessage(IMMessagePtr m)
	{
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1,m->GetText().c_str()); // todo: remove overflow danger
		args[0] = buf1;

		std::string method = "CSendChat";
		std::string syntax = "s";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);
		//todo: Update message history
	}

	void TPIMSession::NotifyMessageReceived(TPIMMessagePtr m)
	{
		message_history_.push_back(m);
	}

} // end of namespace: Communication



// For windows compatibility
#ifdef SendMessageDefined
#define SendMessage SendMessageW
#endif // SendMessageDefined
