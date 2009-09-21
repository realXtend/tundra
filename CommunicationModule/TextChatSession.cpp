#include "TextChatSession.h"

namespace TpQt4Communication
{
	TextChatSession::TextChatSession()
	{


	}

	TextChatSession::TextChatSession(Tp::TextChannelPtr tp_text_channel)
	{
		//tp_text_channel_ = tp_text_channel;
		//QObject::connect(tp_text_channel->becomeReady(), 
		//	SIGNAL( finished(Tp::PendingOperation*) ),
		//	SLOT( OnIncomingTextChannelReady(Tp::PendingOperation*) );
	}

	//void TextChatSession::OnIncomingTextChannelReady(Tp::PendingOperation* op )
	//{
	//	if (op->isError())
	//	{
	//		return;
	//	}

	//	Tp::PendingReady *pr = qobject_cast<PendingReady *>(op);
	//	Tp::TextChannelPtr tp_text_channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));

	//	Tp::ContactPtr initiator = tp_text_channel->initiatorContact();
	//	// tp_text_channel->requestClose(); // CLOSE
	//	//tp_text_channel->
	//}

	void TextChatSession::Invite(Address a)
	{

	}

	void TextChatSession::SendTextMessage(std::string text)
	{
		Message* m = new Message(text);
		messages_.push_back(m);
	}

	MessageVector TextChatSession::GetMessageHistory()
	{
		return messages_;
	}

	void TextChatSession::Close()
	{
		// todo: 
	}

	void TextChatSession::OnChannelReady(Tp::PendingOperation* op)
	{

	}
	
	void TextChatSession::OnChannelInvalidated(Tp::DBusProxy *p, const QString &me, const QString &er)
	{

	}

//	TextChatSessionRequest::TextChatSessionRequest(Tp::TextChannelPtr tp_text_channel)
	TextChatSessionRequest::TextChatSessionRequest(TextChatSessionWeakPtr session ) //Tp::TextChannelPtr tp_text_channel);
	{
		session_ = session;
		//tp_text_channel_ = tp_text_channel;
	}


} // end of namespace:  TpQt4Communication
