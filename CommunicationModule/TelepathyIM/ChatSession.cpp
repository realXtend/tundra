#include "ChatSession.h"

namespace TelepathyIM
{
	ChatSession::ChatSession(Tp::TextChannelPtr tp_text_channel) : tp_text_channel_(tp_text_channel), state_(STATE_INITIALIZING)
	{
		Tp::Features features;
		features.insert(Tp::TextChannel::FeatureMessageQueue);
		features.insert(Tp::TextChannel::FeatureCore);
		features.insert(Tp::TextChannel::FeatureMessageCapabilities);
		connect(tp_text_channel_->becomeReady(features), SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnTextChannelReady(Tp::PendingOperation*)) );
	}

	ChatSession::~ChatSession()
	{
		//! @todo IMPLEMENT

		if (state_ == STATE_OPEN)
			Close();
	}

	void ChatSession::SendMessage(const QString &text)
	{
		if (state_ == STATE_CLOSED)
			throw Core::Exception("Channel closed");

		if (state_ == STATE_INITIALIZING)
		{
			send_buffer_.push_back(text);
			return;
		}

		Tp::PendingSendMessage* p = tp_text_channel_->send( text );

		//! @todo Connect this signal to actual message object
		connect(p, SIGNAL( finished(Tp::PendingOperation* operation) ), SLOT( OnMessageSendAck(Tp::PendingOperation*) ));
	}

	Communication::ChatSessionInterface::State ChatSession::GetState() const
	{
		return state_;
	}

	void ChatSession::Close()
	{
		//! @todo IMPLEMENT
	}

	Communication::ChatSessionParticipantVector ChatSession::GetParticipants() const
	{
		Communication::ChatSessionParticipantVector participants;
		for (ChatSessionParticipantVector::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
		{
			participants.push_back( *i );
		}

		return participants;
	}

	Communication::ChatMessageVector ChatSession::GetMessageHistory()
	{
		Communication::ChatMessageVector messages;
		for (ChatMessageVector::iterator i = message_history_.begin(); i != message_history_.end(); ++i)
		{
			messages.push_back( *i );
		}
		return messages;
	}

	void ChatSession::OnTextChannelCreated(Tp::PendingOperation* op)
	{
		//if ( op->isError() )
		//{
		//	LogError("Cannot create TextChannel object");
		//	return;
		//}
		//LogInfo("TextChannel object created");

		//Tp::PendingChannel *pChannel = qobject_cast<Tp::PendingChannel *>(op);
		//Tp::ChannelPtr text_channel = pChannel->channel();

		//tp_text_channel_ = Tp::TextChannelPtr( dynamic_cast<Tp::TextChannel *>(text_channel.data()) );

		//QObject::connect(tp_text_channel_->becomeReady(Tp::TextChannel::FeatureMessageQueue), 
		//	     SIGNAL( finished(Tp::PendingOperation*) ),
		//		SLOT( OnChannelReady(Tp::PendingOperation*)) );
	}
	
	void ChatSession::OnTextChannelReady(Tp::PendingOperation* op)
	{
		for (QStringList::iterator i = send_buffer_.begin(); i != send_buffer_.end(); ++i)
		{
			SendMessage( *i);
		}
	}

	void ChatSession::OnMessageSendAck(Tp::PendingOperation* op)
	{
		//! @todo Move this slot to actual message object
		//! @todo IMPLEMENT
	}

} // end of namespace: TelepathyIM
