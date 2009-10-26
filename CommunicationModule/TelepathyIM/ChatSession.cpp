#include "ChatSession.h"
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/constants.h>

namespace TelepathyIM
{
	ChatSession::ChatSession(Contact& contact, Tp::ConnectionPtr tp_connection) : state_(STATE_INITIALIZING), self_participant_(NULL)
	{
		ChatSessionParticipant* p = new ChatSessionParticipant(&contact);
		participants_.push_back(p);

		QVariantMap params;
		params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"), QLatin1String(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
		params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"), Tp::HandleTypeContact);
		params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"), contact.GetTpContact()->handle().at(0));

		Tp::PendingChannel* pending_channel = tp_connection->ensureChannel(params);
		connect(pending_channel,
	   			SIGNAL( finished(Tp::PendingOperation*) ),
				SLOT( OnTextChannelCreated(Tp::PendingOperation*) ));
	}

	ChatSession::ChatSession(Contact& initiator, Tp::TextChannelPtr tp_text_channel) : tp_text_channel_(tp_text_channel), state_(STATE_INITIALIZING), self_participant_(NULL)
	{
		ChatSessionParticipant* p = new ChatSessionParticipant(&initiator);
		participants_.push_back(p);

		Tp::Features features;
		features.insert(Tp::TextChannel::FeatureMessageQueue);
		features.insert(Tp::TextChannel::FeatureCore);
		features.insert(Tp::TextChannel::FeatureMessageCapabilities);
		connect(tp_text_channel_->becomeReady(features), SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnTextChannelReady(Tp::PendingOperation*)) );
	}

	ChatSession::ChatSession(const QString &room_id, Tp::ConnectionPtr tp_connection): state_(STATE_INITIALIZING), self_participant_(NULL)
	{
		QVariantMap params;
		params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"), QLatin1String(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
		params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"), Tp::HandleTypeRoom);
		params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetID"), room_id);

		Tp::PendingChannel* pending_channel = tp_connection->ensureChannel(params);
		connect(pending_channel,
	   			SIGNAL( finished(Tp::PendingOperation*) ),
				SLOT( OnTextChannelCreated(Tp::PendingOperation*) ));
	}

	ChatSession::~ChatSession()
	{
		if (state_ == STATE_OPEN)
			Close();

		for (ChatMessageVector::iterator i = message_history_.begin(); i != message_history_.end(); ++i)
		{
			SAFE_DELETE(*i);
		}
		message_history_.clear();
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

		ChatMessage* m = new ChatMessage(&self_participant_, QDateTime::currentDateTime(), text);
		message_history_.push_back(m);

		Tp::PendingSendMessage* p = tp_text_channel_->send( text );

		//! @todo Connect this signal to actual message object
		connect(p, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnMessageSendAck(Tp::PendingOperation*) ));
	}

	Communication::ChatSessionInterface::State ChatSession::GetState() const
	{
		return state_;
	}

	void ChatSession::Close()
	{
		if ( tp_text_channel_.isNull() )
		{
			state_ = STATE_CLOSED; 
			return;
		}
		Tp::PendingOperation* op = tp_text_channel_->requestClose(); 
		connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnTextChannelClosed(Tp::PendingOperation*) ));
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

	ChatSessionParticipant* ChatSession::GetParticipant(Tp::ContactPtr contact)
	{
		for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
		{
			if ( (*i)->GetID().compare( contact->id() ) == 0)
				return *i;
		}
		return 0;
	}

	ChatSessionParticipant* ChatSession::GetParticipant(uint sender_id)
	{
		for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
		{
			
			if ( (*i)->GetID().compare(QString(sender_id)) == 0)
				return *i;
		}
		return 0;
	}

	void ChatSession::OnTextChannelCreated(Tp::PendingOperation* op)
	{
		if ( op->isError() )
		{
			state_ = STATE_ERROR;	
			return;
		}
		
		Tp::PendingChannel *pChannel = qobject_cast<Tp::PendingChannel *>(op);
		tp_text_channel_ = Tp::TextChannelPtr(dynamic_cast<Tp::TextChannel *>( pChannel->channel().data() ));

		connect(tp_text_channel_->becomeReady(),
			    SIGNAL( finished(Tp::PendingOperation*) ),
				SLOT( OnTextChannelReady(Tp::PendingOperation*) ));
	}

	void ChatSession::OnChannelInvalidated(Tp::DBusProxy *p, const QString &me, const QString &er)
	{
		//! @todo IMPLEMENT	

		//! When does this happen?
	}
	
	void ChatSession::OnTextChannelReady(Tp::PendingOperation* op)
	{
		if (op->isError())
		{
			state_ = STATE_ERROR;
			return;
		}

		//Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
		//Tp::TextChannelPtr channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));
		//tp_text_channel_ = channel;

		QStringList interfaces = tp_text_channel_->interfaces();
		for (QStringList::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
		{
			QString line = "Text channel have interface: ";
			line.append(*i);
			LogDebug(line.toStdString());
		}

		//! @todo Store the value 
		Tp::ContactPtr initiator = tp_text_channel_->initiatorContact();

		//connect(tp_text_channel_.data(),
		//	    SIGNAL( messageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &) ), 
  //              SLOT( OnChannelMessageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &) ));

		connect(tp_text_channel_.data(),
				SIGNAL( messageReceived(const Tp::ReceivedMessage &) ),
				SLOT( OnMessageReceived(const Tp::ReceivedMessage &) ));

		//connect(tp_text_channel_.data(), 
		//		SIGNAL( pendingMessageRemoved(const Tp::ReceivedMessage &) ), 
		//	    SLOT( OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &) ));


		HandlePendingMessage();
		state_ = STATE_OPEN;
		emit Opened(this);
		for (QStringList::iterator i = send_buffer_.begin(); i != send_buffer_.end(); ++i)
			SendMessage( *i);
	}

	void ChatSession::HandlePendingMessage()
	{
	    QDBusPendingReply<Tp::PendingTextMessageList> pending_messages = tp_text_channel_->textInterface()->ListPendingMessages(true);
		
		if( !pending_messages.isFinished() )
			pending_messages.waitForFinished();
		if ( pending_messages.isValid() )
		{
			LogDebug("Received pending messages:");
			QDBusMessage m = pending_messages.reply();
			Tp::PendingTextMessageList list = pending_messages.value();
			
			for (Tp::PendingTextMessageList::iterator i = list.begin(); i != list.end(); ++i)
			{
				QString note = QString("* Pending message received: ").append(i->text);
				LogDebug(note.toStdString());

				Core::uint type = i->messageType; //! @todo Check if we need value of this parameter

				ChatMessage* message = new ChatMessage( GetParticipant(i->sender), QDateTime::fromTime_t(i->unixTimestamp), i->text);
				message_history_.push_back(message);
				emit( MessageReceived(*message) );
			}
		}
		else
			LogError("Received invalid pending messages");
	}

	void ChatSession::OnMessageSendAck(Tp::PendingOperation* op)
	{
		//! @todo Move this slot to actual message object
		//! @todo IMPLEMENT
	}

	void ChatSession::OnMessageReceived(const Tp::ReceivedMessage &message)
	{
		ChatSessionParticipant* from = GetParticipant(message.sender());
		ChatMessage* m = new ChatMessage(from, message.received(), message.text());
		message_history_.push_back(m);
		emit( MessageReceived(*m) );
	}

	void ChatSession::OnTextChannelClosed(Tp::PendingOperation* op)
	{
		state_ = STATE_CLOSED;
		emit( Closed(this) );
	}

} // end of namespace: TelepathyIM
