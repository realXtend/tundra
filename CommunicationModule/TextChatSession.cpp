#include "TextChatSession.h"

namespace TpQt4Communication
{


	TextChatSession::TextChatSession(): tp_text_channel_(NULL), state_(STATE_INITIALIZING)
	{

	}

	TextChatSession::TextChatSession(Tp::TextChannelPtr tp_text_channel): tp_text_channel_(tp_text_channel), state_(STATE_INITIALIZING)
	{
		QObject::connect(tp_text_channel_->becomeReady(), 
					     SIGNAL( finished(Tp::PendingOperation*) ),
						// SLOT( OnIncomingTextChannelReady(Tp::PendingOperation*)) );
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
		if (op->isError())
		{
			//op->Message();
			LogError("Cannot initialize text channel");
			return;
		}

		LogInfo("TextChatSession->OnChannelReady");
		Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
		Tp::TextChannelPtr tp_text_channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));

		Tp::ContactPtr initiator = tp_text_channel->initiatorContact();
		// tp_text_channel->requestClose(); // CLOSE
		//tp_text_channel->

		QObject::connect(tp_text_channel_.data(),
						 SIGNAL( messageReceived(const Tp::ReceivedMessage &)),
						 SLOT( OnMessageReceived(const Tp::ReceivedMessage &) ) );
	}

	void TextChatSession::Invite(Address a)
	{

	}

	void TextChatSession::SendTextMessage(std::string text)
	{
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
		LogInfo("Try to close chat sessions.");
	//	tp_text_channel_->requestClose();
		// todo: 
	}

	
	void TextChatSession::OnChannelInvalidated(Tp::DBusProxy *p, const QString &me, const QString &er)
	{

	}

	void TextChatSession::OnMessageReceived(const Tp::ReceivedMessage &message)
	{
		Message* m = new Message("");
		messages_.push_back(m);
		LogInfo("Received text message");
	}

	void TextChatSession::OnTextChannelCreated(Tp::PendingOperation* op)
	{
		if ( op->isError() )
		{
			LogError("Cannot create TextChannel object");
			return;
		}

		LogInfo("TextChannel object -> Created");

		Tp::PendingChannel *pChannel = qobject_cast<Tp::PendingChannel *>(op);
		Tp::ChannelPtr text_channel = pChannel->channel();
		
//		Tp::Channel *channel = text_channel.data();  
//		Tp::TextChannel *textChannel = dynamic_cast<Tp::TextChannel *>(channel);
		tp_text_channel_ = Tp::TextChannelPtr( dynamic_cast<Tp::TextChannel *>(text_channel.data()) );
		state_ = STATE_READY;
		emit Ready();
	}

	TextChatSession::State TextChatSession::GetState()
	{
		
		return state_;
	}














} // end of namespace:  TpQt4Communication
