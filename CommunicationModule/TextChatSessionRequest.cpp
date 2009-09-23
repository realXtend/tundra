#include "TextChatSessionRequest.h"

namespace TpQt4Communication
{
	TextChatSessionRequest::TextChatSessionRequest(TextChatSession* session ): session_(session), message_("")
	{
		//tp_text_channel_ = tp_text_channel;
	}

	TextChatSessionRequest::TextChatSessionRequest(Tp::TextChannelPtr tp_text_channel)
	{
		tp_text_channel_ = tp_text_channel;
		session_ = new TextChatSession(tp_text_channel);
	}

	TextChatSession* TextChatSessionRequest::Accept()
	{
//		TextChatSession* session = new TextChatSession(tp_text_channel_);
		return session_;
	}

	void TextChatSessionRequest::Reject()
	{
		Tp::PendingOperation* p = tp_text_channel_->requestClose();
		//QObject::connect(p,
		//	SIGNAL( finished(Tp::PendingOperation* ),
		//	SLOT( OnTextChannelClosed(Tp::PendingOperation* ) );
		//! todo: close channel
		//tp_text_channel_
	}

	Address TextChatSessionRequest::GetOriginator()
	{
		//! todo: implement
		return "NOT IMPLEMENTED."; 
	}

	std::string TextChatSessionRequest::GetMessage()
	{
		return message_;
	}

	void TextChatSessionRequest::OnTextChannelClosed(Tp::PendingOperation* op)
	{
	//	LogInfo("Text channel closed");
	}

} // end of namespace: TpQt4Communication
