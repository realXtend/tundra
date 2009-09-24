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

	void TextChatSession::OnChannelReady(Tp::PendingOperation* op )
	{
		Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
		Tp::TextChannelPtr channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));

		if (channel.data() != tp_text_channel_.data() )
		{
			LogError(">>>>>>>>>>>>>>>>> Tp:TextChannel object is changed!!!!");
		}

		if (tp_text_channel_.isNull())
		{
			LogError("TextChatSession::OnChannelReady - tp_text_channel_ == NULL");
			return;
		}

		if (op->isError())
		{
			//op->Message();
			LogError("Cannot initialize text channel");
			return;
		}

		LogInfo("Text channel ready.");
//		Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
//		Tp::TextChannelPtr tp_text_channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));
//		tp_text_channel_ = tp_text_channel;

		Tp::ContactPtr initiator = tp_text_channel_->initiatorContact();
		if ( !initiator.isNull() )
			this->originator_ =	initiator->id().toStdString();
		else
			LogError("channel initiator == NULL");

		QObject::connect(tp_text_channel_.data(),
						 SIGNAL( messageReceived(const Tp::ReceivedMessage &)),
						 SLOT( OnMessageReceived(const Tp::ReceivedMessage &) ) );
		//QObject::connect(tp_text_channel_.data(),
		//				 SIGNAL( onTextReceived(uint a, uint b, uint c, uint d, uint e, const QString &text)),
		//				 SLOT( OnTextReceived(uint a, uint b, uint c, uint d, uint e, const QString &text) ) );
		LogInfo("connected signal: tp_text_channel_->OnMessageReceived");
		state_ = STATE_READY;
		emit Ready();
	}

	void TextChatSession::OnTextReceived(uint a, uint b, uint c, uint d, uint e, const QString &text)
	{
		LogError("tp_text_channel_->OnTextReceived");
	}

	void TextChatSession::Invite(Address a)
	{
		LogError("NOT IMPLEMENTED.");
	}

	void TextChatSession::SendTextMessage(std::string text)
	{
		assert( tp_text_channel_.isNull() );
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

	void TextChatSession::OnMessageReceived(const Tp::ReceivedMessage &message)
	{
		Message* m = new Message(message.text().toStdString());
		messages_.push_back(m);
		emit MessageReceived(*m);
		LogInfo("Received text message");
	}

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
		
//		Tp::Channel *channel = text_channel.data();  
//		Tp::TextChannel *textChannel = dynamic_cast<Tp::TextChannel *>(channel);
		tp_text_channel_ = Tp::TextChannelPtr( dynamic_cast<Tp::TextChannel *>(text_channel.data()) );

		QObject::connect(tp_text_channel_->becomeReady(), 
			     SIGNAL( finished(Tp::PendingOperation*) ),
				SLOT( OnChannelReady(Tp::PendingOperation*)) );

	}

	TextChatSession::State TextChatSession::GetState()
	{
		return state_;
	}

	//void TextChatSession::OnTextChannelReady(Tp::PendingOperation *op)
	//{
	//	LogInfo("->OnTextChannelReady");
	//}










} // end of namespace:  TpQt4Communication
