#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationManager.h"
#include "IMSession.h"

namespace Communication
{

	Session::Session(ParticipantPtr originator, Foundation::ScriptObjectPtr python_communication_object) : originator_(originator), protocol_("")
	{
		python_communication_object_ = python_communication_object;		
		participants_ = ParticipantListPtr( new ParticipantList() );
	}

	//std::string Session::GetId()
	//{
	//	return id_;
	//}

	std::string Session::GetProtocol()
	{
		return protocol_;
	}

	ParticipantPtr Session::GetOriginator()
	{
		return originator_;
	}

	void Session::Close()
	{
	}

	void Session::NotifyClosedByRemote()
	{
		// todo: handle this
	}


	void Session::SendInvitation(ContactPtr c)
	{
		// Not implemented in python yet: Multiuser chat
	}

	void Session::Kick(ParticipantPtr p)
	{
		// Not implemented in python yet:
	}

	ParticipantListPtr Session::GetParticipants()
	{
		return participants_;
	}

	// IMSession ---------------------------->

	IMSession::IMSession(ParticipantPtr originator, Foundation::ScriptObjectPtr python_communication_object) : Session(originator, python_communication_object)
	{
	}

	/*
	 * Send IM message to all person in current session
	 */
	void IMSession::SendIMMessage(IMMessagePtr m)
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
			strcpy(buf1, arg_text.c_str() ); 
			args[0] = buf1;

			std::string method = "CSendChat";
			std::string syntax = "s";
			Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);
		}

		im_messages_.push_back(m);
	}

	// Update message history 
	void IMSession::NotifyMessageReceived(IMMessagePtr m)
	{
		im_messages_.push_back(m);
	}
	
	IMMessageListPtr IMSession::GetMessageHistory()
	{
		IMMessageList* list = new IMMessageList();
		for (int i = 0; i < im_messages_.size(); i++)
		{
			list->push_back(im_messages_[i]);
		}
		return IMMessageListPtr(list);
	}

	void IMSession::Close()
	{
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, id_.c_str());
		args[0] = buf1;
		std::string method = "CCloseChannel";
		std::string syntax = "s";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);

		CommunicationManager::GetInstance()->RemoveIMSession(this);
	}

	void IMSession::SendInvitation(ContactPtr c)
	{
		Session::SendInvitation(c);
	}

	void IMSession::Kick(ParticipantPtr p)
	{
		Session::Kick(p);
	}

	ParticipantListPtr IMSession::GetParticipants()
	{
		return Session::GetParticipants();
	}

	std::string IMSession::GetProtocol()
	{
		return Session::GetProtocol();
	}
	
	ParticipantPtr IMSession::GetOriginator()
	{
		return Session::GetOriginator();
	}




} // end of namespace: Communication
