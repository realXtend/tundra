#include "StableHeaders.h"
#include "Foundation.h"

#include "TPIMSession.h"

namespace Communication
{
	TPSession::TPSession(Foundation::ScriptObjectPtr python_communication_object)
	{
		python_communication_object_ = python_communication_object;		
		participients_ = ParticipientListPtr( new ParticipientList() );
	}

	std::string TPSession::GetId()
	{
		return id_;
	}

	void TPSession::Close()
	{
		std::string method = "CCloseChannel";
		std::string syntax = "";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
		// todo: inform TelepathyCommunication about end of session
		// todo: where to remove session from list ???
	}

	void TPSession::NotifyClosedByRemote()
	{
		// todo: handle this
	}


	void TPSession::SendInvitation(ContactPtr c)
	{
		// Not implemented in python yet: Multiuser chat
	}

	void TPSession::Kick(Participiant *p)
	{
		// Not implemented in python yet:
	}

	ParticipientListPtr TPSession::GetParticipients()
	{
		return participients_;
	}

	TPIMSession::TPIMSession(Foundation::ScriptObjectPtr python_communication_object):TPSession(python_communication_object)
	{
	}

	void TPIMSession::SendIMMessage(IMMessagePtr m)
	{
		char** args = new char*[1];
		char* buf1 = new char[1000];
		std::string text = m->GetText();
		((TPIMMessage*)m.get())->session_id_ = id_;
		strcpy(buf1, text.c_str() ); // todo: remove overflow danger
		args[0] = buf1;

		std::string method = "CSendChat";
		std::string syntax = "s";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);
		im_messages_.push_back(m);
	}

	void TPIMSession::NotifyMessageReceived(IMMessagePtr m)
	{
		im_messages_.push_back(m);
	}
	
	IMMessageListPtr TPIMSession::GetMessageHistory()
	{
		IMMessageList* list = new IMMessageList();
		for (int i = 0; i < im_messages_.size(); i++)
		{
			list->push_back(im_messages_[i]);
		}
		return IMMessageListPtr(list);
	}


} // end of namespace: Communication
