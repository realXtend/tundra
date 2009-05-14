#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationManager.h"
#include "IMSession.h"

namespace Communication
{

	Session::Session(ParticipantPtr originator) : originator_(originator), protocol_("")
	{
		participants_ = ParticipantListPtr( new ParticipantList() );
	}

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
		// todo: handle this: how?
	}

	void Session::SendInvitation(ContactPtr c)
	{
		// Not implemented in python yet (Multiuser chat)
	}

	void Session::Kick(ParticipantPtr p)
	{
		// Not implemented in python yet (Multiuser chat)
	}

	ParticipantListPtr Session::GetParticipants()
	{
		return participants_;
	}

	// todo: Move this to own file
	// IMSession ---------------------------->

	IMSession::IMSession(ParticipantPtr originator) : Session(originator)
	{
	}

	/**
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
				// @todo Report this!
				return;
			}

			std::string arg_text;
			arg_text.append(address);
			arg_text.append(":");
			arg_text.append(m->GetText());
			CommunicationManager::GetInstance()->CallPythonCommunicationObject("CSendChat", arg_text);
		}

		im_messages_.push_back(m);
	}

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
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CCloseChannel", id_);
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
