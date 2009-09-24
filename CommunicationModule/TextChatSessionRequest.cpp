#include "TextChatSessionRequest.h"

namespace TpQt4Communication
{
	TextChatSessionRequest::TextChatSessionRequest(TextChatSession* session ): session_(session), message_("")
	{
		//tp_text_channel_ = tp_text_channel;
	}

	TextChatSessionRequest::TextChatSessionRequest(Tp::TextChannelPtr tp_text_channel, Contact* from): from_(from)
	{
		tp_text_channel_ = tp_text_channel;
		session_ = new TextChatSession(tp_text_channel);
	}

	TextChatSessionPtr TextChatSessionRequest::Accept()
	{
		LogInfo("Chat session accepted.");
//		TextChatSession* session = new TextChatSession(tp_text_channel_);
		return TextChatSessionPtr(session_);
	}

	void TextChatSessionRequest::Reject()
	{
		LogInfo("Chat session rejected.");
		Tp::PendingOperation* p = tp_text_channel_->requestClose();
		QObject::connect(p,
			SIGNAL( finished(Tp::PendingOperation* ) ),
			SLOT( OnTextChannelClosed(Tp::PendingOperation* ) ));
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
		if ( op->isError())
		{
			LogInfo("Cannot close text channel.");
		}
		LogInfo("Text channel closed.");
	}

	Contact* TextChatSessionRequest::GetOriginatorContact()
	{
		//Tp::ContactPtr c =  tp_text_channel_->initiatorContact();
		return from_; 
	}


} // end of namespace: TpQt4Communication
