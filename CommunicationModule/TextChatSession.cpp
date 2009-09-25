#include "TextChatSession.h"

namespace TpQt4Communication
{


	TextChatSession::TextChatSession(): tp_text_channel_(NULL), state_(STATE_INITIALIZING)
	{
		LogInfo("TextChatSession object created.");
	}

	TextChatSession::TextChatSession(Tp::TextChannelPtr tp_text_channel): tp_text_channel_(tp_text_channel), state_(STATE_INITIALIZING)
	{
		LogInfo("TextChatSession object created (with channel object)");
		Tp::Features features;
		features.insert(Tp::TextChannel::FeatureMessageQueue);
		features.insert(Tp::TextChannel::FeatureCore);
		features.insert(Tp::TextChannel::FeatureMessageCapabilities);
		QObject::connect(tp_text_channel_->becomeReady(features), 
					     SIGNAL( finished(Tp::PendingOperation*) ),
						SLOT( OnChannelReady(Tp::PendingOperation*)) );
	}

	TextChatSession::~TextChatSession()
	{
		for (MessageVector::iterator i = messages_.begin(); i != messages_.end(); ++i)
		{
			delete *i;
		}
		messages_.clear();
	}

	void TextChatSession::SendTextMessage(std::string text)
	{
		assert( !tp_text_channel_.isNull() );

		Message* m = new Message(text, NULL);
		messages_.push_back(m);

		Tp::PendingSendMessage* p = tp_text_channel_->send(QString(text.c_str()));
		//! todo: receive ack when message is send
	}

	MessageVector TextChatSession::GetMessageHistory()
	{
		return messages_;
	}

	void TextChatSession::Close()
	{
		assert( tp_text_channel_.isNull() );
		LogInfo("Try to close chat sessions.");
		tp_text_channel_->requestClose(); 
		//! todo: connect signal from pending operation
	}

	
	void TextChatSession::OnChannelInvalidated(Tp::DBusProxy *p, const QString &me, const QString &er)
	{
		LogInfo("TextChatSession->OnChannelInvalidated");
	}

	//! This method will not be called if session received from IM server this only aplied sessions 
	//! created by Connection::CreateTextChatSession
	void TextChatSession::OnTextChannelCreated(Tp::PendingOperation* op)
	{
		if ( op->isError() )
		{
			LogError("Cannot create TextChannel object");
			return;
		}
		LogInfo("TextChannel object created");

		Tp::PendingChannel *pChannel = qobject_cast<Tp::PendingChannel *>(op);
		Tp::ChannelPtr text_channel = pChannel->channel();

		tp_text_channel_ = Tp::TextChannelPtr( dynamic_cast<Tp::TextChannel *>(text_channel.data()) );

		QObject::connect(tp_text_channel_->becomeReady(Tp::TextChannel::FeatureMessageQueue), 
			     SIGNAL( finished(Tp::PendingOperation*) ),
				SLOT( OnChannelReady(Tp::PendingOperation*)) );
	}

	void TextChatSession::OnChannelReady(Tp::PendingOperation* op )
	{
		Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
		Tp::TextChannelPtr channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));
		tp_text_channel_ = channel;

		if (op->isError())
		{
			QString e = "Cannot initialize text channel: ";
			e.append( op->errorMessage() );
			LogError( e.toStdString() );
			state_ = STATE_ERROR;
			return;
		}
		LogInfo("Text channel ready.");

		bool ready1 = tp_text_channel_->isReady(Tp::TextChannel::FeatureMessageCapabilities);
	    bool ready2 = tp_text_channel_->isReady(Tp::TextChannel::FeatureMessageQueue);
	    bool ready3 = tp_text_channel_->isReady(Tp::TextChannel::FeatureMessageSentSignal);

		QStringList interfaces = tp_text_channel_->interfaces();
		for (QStringList::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
		{
			QString line = "Text channel have interface: ";
			line.append(*i);
			LogInfo(line.toStdString());
		}

		Tp::ContactPtr initiator = tp_text_channel_->initiatorContact();
		if ( !initiator.isNull() )
			this->originator_ =	initiator->id().toStdString();
		else
			LogError("channel initiator == NULL");

		connect((QObject*)tp_text_channel_.data(), 
            SIGNAL(messageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &)), 
            SLOT(OnChannelMessageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &)));

		QObject::connect(tp_text_channel_.data(),
						 SIGNAL( messageReceived(const Tp::ReceivedMessage &)),
						 SLOT( OnMessageReceived(const Tp::ReceivedMessage &) ) );

		connect(tp_text_channel_.data(), 
				SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage &)), 
			    SLOT(OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &)));

		// Receive pending messages
	    QDBusPendingReply<Tp::PendingTextMessageList> pending_messages = tp_text_channel_->textInterface()->ListPendingMessages(true);
		
		if( !pending_messages.isFinished() )
			pending_messages.waitForFinished();
		if ( pending_messages.isValid() )
		{
			LogInfo("Received pending message");
			QDBusMessage m = pending_messages.reply();
			Tp::PendingTextMessageList list = pending_messages.value();
			
			for (Tp::PendingTextMessageList::iterator i = list.begin(); i != list.end(); ++i)
			{
				QString text = i->text;
				Core::uint s = i->sender;
				Core::uint t = i->unixTimestamp;
				Message* m = new Message(text.toStdString(), new Contact(tp_text_channel_->initiatorContact()));
				messages_.push_back(m);
				emit MessageReceived(*m);
				LogInfo("emited pending message");
			}
		}
		else
			LogError("Received invalid pending message");

		LogInfo("TextChatSession object ready.");
		state_ = STATE_READY;
		emit Ready();
	}

	void TextChatSession::OnChannelMessageSent(const Tp::Message& message, Tp::MessageSendingFlags flags, const QString &text)
	{
		LogInfo("OnChannelMessageSent");
	}

	void TextChatSession::OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &message)
	{
		LogInfo("OnChannelPendingMessageRemoved");
	}

	void TextChatSession::OnMessageReceived(const Tp::ReceivedMessage &message)
	{
		Tp::ContactPtr sender = message.sender();
		Message* m = new Message(message.text().toStdString(), new Contact(sender));
		messages_.push_back(m);
		emit MessageReceived(*m);
		LogInfo("Received text message");
	}

	TextChatSession::State TextChatSession::GetState()
	{
		return state_;
	}

} // end of namespace:  TpQt4Communication
