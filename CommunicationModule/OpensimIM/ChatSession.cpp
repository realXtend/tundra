#include "ChatSession.h"
#include "RexLogicModule.h" 
#include "OpensimProtocolModule.h"

namespace OpensimIM
{
	ChatSession::ChatSession(Foundation::Framework* framework, const QString &channel_id): framework_(framework), channel_id_(channel_id), server_participant_("0", "Server"), private_im_session_(false)
	{
		//! \todo Add support to different channel numbers
		//!       This requires changes to SendChatFromViewerPacket method or 
		//!       chat packet must be construaed by hand.
		if ( channel_id.compare("0") != 0 )
			throw Core::Exception("Cannot create chat session, channel id now allowed"); 
	}

	ChatSession::ChatSession(Foundation::Framework* framework): framework_(framework), channel_id_(""), server_participant_("0", "Server"), private_im_session_(true)
	{
	}

	void ChatSession::SendMessage(const QString &text)
	{
		if (private_im_session_)
			SendPrivateIMMessage(text);
		else
			SendPublicChatMessage(text);

	}

	void ChatSession::SendPrivateIMMessage(const QString &text)
	{
		RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

		if (rexlogic_ == NULL)
			throw Core::Exception("Cannot send IM message, RexLogicModule is not found");
		RexLogic::RexServerConnectionPtr connection = rexlogic_->GetServerConnection();

		if ( connection == NULL )
			throw Core::Exception("Cannot send IM message, rex server connection is not found");

		if ( !connection->IsConnected() )
			throw Core::Exception("Cannot send IM message, rex server connection is not established");


		for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
		{
			connection->SendImprovedInstantMessagePacket(RexTypes::RexUUID( (*i)->GetID().toStdString() ), text.toStdString() );
		}
	}

	void ChatSession::SendPublicChatMessage(const QString &text)
	{
		RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

		if (rexlogic_ == NULL)
			throw Core::Exception("Cannot send text message, RexLogicModule is not found");
		RexLogic::RexServerConnectionPtr connection = rexlogic_->GetServerConnection();

		if ( connection == NULL )
			throw Core::Exception("Cannot send text message, rex server connection is not found");

		if ( !connection->IsConnected() )
			throw Core::Exception("Cannot send text message, rex server connection is not established");

		connection->SendChatFromViewerPacket( text.toStdString() );
	}

	void ChatSession::Close()
	{
		//! \todo IMPLEMENT
	}
	void ChatSession::MessageFromAgent(const QString &avatar_id, const QString &name, const QString &text)
	{
		ChatSessionParticipant* participant = FindParticipant(avatar_id);
		if ( !participant )
		{
			participant = new ChatSessionParticipant(avatar_id, name);
			participants_.push_back(participant);
		}

		emit MessageReceived(text, *participant);

		//! @note For testing...
		//OpenSimProtocol::OpenSimProtocolModule *opensim_protocol_ = dynamic_cast<OpenSimProtocol::OpenSimProtocolModule*>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol).lock().get());
		//if ( opensim_protocol_ )
		//{
		//	if ( opensim_protocol_->GetClientParameters().agentID.ToString().compare( avatar_id.toStdString() ) != 0 )
		//	{
		//		QString message = "ECHO: ";
		//		message.append(text);
		//		SendMessage(message);
		//	}
		//}
	}

	void ChatSession::MessageFromServer(const QString &text)
	{
		emit MessageReceived(text, dynamic_cast<Communication::ChatSessionParticipantInterface&>(server_participant_));
	}

	void ChatSession::MessageFromObject(const QString &object_id, const QString &text)
	{
		ChatSessionParticipant* participant = FindParticipant(object_id);
		if ( !participant )
		{
			ChatSessionParticipant* participant = new ChatSessionParticipant(object_id, "(name)");
			participants_.push_back(participant);
		}

		emit MessageReceived(text, *participant);
	}

	ChatSessionParticipant* ChatSession::FindParticipant(const QString &uuid)
	{
		for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
		{
			if ( (*i)->GetID().compare(uuid) == 0 )
				return *i;
		}
		return NULL;
	}

	Communication::ChatSessionParticipantVector ChatSession::GetParticipants() const
	{
		Communication::ChatSessionParticipantVector participants;
		for (ChatSessionParticipantVector::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
		{
			participants.push_back(*i);
		}
		return participants;
	}

	QString ChatSession::GetID() const
	{
		return channel_id_;
	}

	bool ChatSession::IsPrivateIMSession()
	{
		return private_im_session_;
	}

} // end of namespace: OpensimIM
