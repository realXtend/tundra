#include "StableHeaders.h"
#include "Foundation.h"
#include "TelepathyCommunication.h"
#include "TPIMSession.h"

namespace Communication
{

	TPSession::TPSession(Foundation::ScriptObjectPtr python_communication_object):protocol_("")
	{
		python_communication_object_ = python_communication_object;		
		participants_ = ParticipantListPtr( new ParticipantList() );
	}

	std::string TPSession::GetId()
	{
		return id_;
	}

	std::string TPSession::GetProtocol()
	{
		return protocol_;
	}

	void TPSession::Close()
	{
		std::string method = "CCloseChannel";
		std::string syntax = "";
		// todo: check if it is safe to send NULL as a last parameter here...
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
		TelepathyCommunication::GetInstance()->RemoveIMSession(id_);
		// todo: When it is possible to have multiple sessions on python side we must send session id here
	}

	void TPSession::NotifyClosedByRemote()
	{
		// todo: handle this
	}


	void TPSession::SendInvitation(ContactPtr c)
	{
		// Not implemented in python yet: Multiuser chat
	}

	void TPSession::Kick(ParticipantPtr p)
	{
		// Not implemented in python yet:
	}

	ParticipantListPtr TPSession::GetParticipants()
	{
		return participants_;
	}

	TPIMSession::TPIMSession(Foundation::ScriptObjectPtr python_communication_object):TPSession(python_communication_object)
	{
//		participants_ = ParticipantListPtr
	}

	/*
	 * Send IM message to all person in current session
	 */
	void TPIMSession::SendIMMessage(IMMessagePtr m)
	{
		for (ParticipantList::iterator i = participants_->begin(); i < participants_->end(); i++)
		{
			ParticipantPtr participant = *i;
			ContactInfoPtr info = participant->GetContact()->GetContactInfo(protocol_);
			std::string address = info->GetProperty("address");
			if ( address.length() == 0 )
			{
				// We have a problem: We don't know address of this participant!
			}

			char** args = new char*[1];
			char* buf1 = new char[1000];
			std::string arg_text;
			arg_text.append(address);
			arg_text.append(":");
			arg_text.append(m->GetText());
		//		((TPIMMessage*)m.get())->session_id_ = id_; // we don't need the session id yet, but might be handy in future
			strcpy(buf1, arg_text.c_str() ); 
			args[0] = buf1;

			std::string method = "CSendChat";
			std::string syntax = "s";
			Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);
		}

		im_messages_.push_back(m);
	}

	// Update message history 
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
