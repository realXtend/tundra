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
		QObject::connect(tp_text_channel_->becomeReady(), 
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


	//void TextChatSession::OnTextReceived(uint a, uint b, uint c, uint d, uint e, const QString &text)
	//{
	//	LogError("tp_text_channel_->OnTextReceived");
	//}

	void TextChatSession::Invite(Address a)
	{
		LogError("NOT IMPLEMENTED.");
	}

	void TextChatSession::SendTextMessage(std::string text)
	{
		assert( !tp_text_channel_.isNull() );
		LogInfo("Try to send a text message.");

		Message* m = new Message(text);
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

	// 
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
//		text_channel_ = text_channel;

		bool ready1 = text_channel->isReady(Tp::TextChannel::FeatureMessageCapabilities);
	    bool ready2 = text_channel->isReady(Tp::TextChannel::FeatureMessageQueue);
	    bool ready3 = text_channel->isReady(Tp::TextChannel::FeatureMessageSentSignal);
		
//		Tp::Channel *channel = text_channel.data();  
//		Tp::TextChannel *textChannel = dynamic_cast<Tp::TextChannel *>(channel);
		tp_text_channel_ = Tp::TextChannelPtr( dynamic_cast<Tp::TextChannel *>(text_channel.data()) );

		QObject::connect(tp_text_channel_->becomeReady(), 
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
			//op->Message();
			LogError("Cannot initialize text channel");
			return;
		}
		LogInfo("Text channel ready.");

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
//		QDBusPendingReply<Tp::PendingTextMessageList>::ite
		
		if( !pending_messages.isFinished() )
		{
			pending_messages.waitForFinished();
		}
		int count = pending_messages.count();
		for (Tp::PendingTextMessageList::iterator i = pending_messages.value().begin(); i != pending_messages.value().end(); ++i)
		{
			//QString text = (*i).text;
		}

		//for(int i = 0; i < count; ++i)
		//{
		//	//Tp::PendingTextMessage m = pending_messages.argumentAt(i);
		//	Tp::PendingTextMessage m = pending_messages.value()[i];
		//	uint sender = m.sender;
		//	QString text = m.text;
		//}

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
		Message* m = new Message(message.text().toStdString());
		messages_.push_back(m);
		emit MessageReceived(*m);
		LogInfo("Received text message");
	}

	TextChatSession::State TextChatSession::GetState()
	{
		return state_;
	}


} // end of namespace:  TpQt4Communication
