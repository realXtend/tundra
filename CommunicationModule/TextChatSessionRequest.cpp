#include "TextChatSessionRequest.h"

namespace TpQt4Communication
{
	//TextChatSessionRequest::TextChatSessionRequest(TextChatSession* session ): session_(session), message_("")
	//{
	//	//tp_text_channel_ = tp_text_channel;
	//}

	TextChatSessionRequest::TextChatSessionRequest(Tp::TextChannelPtr tp_text_channel):  tp_text_channel_(tp_text_channel)
	{
		session_ = new TextChatSession(tp_text_channel);
		Tp::PendingReady* p =  tp_text_channel->becomeReady(Tp::TextChannel::FeatureMessageQueue);
		QObject::connect(p, SIGNAL(finished(Tp::PendingOperation*)), SLOT( OnTextChatSessionReady() ) );
	}

	TextChatSessionPtr TextChatSessionRequest::Accept()
	{
		LogInfo("Chat session accepted.");
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
		LogInfo("TextChatSessionRequest::GetOriginatorContact()");
		if ( !tp_text_channel_->isReady() )
		{
			LogError("tp_text_channel_ == NULL");
			return NULL;
		}
		return new Contact(tp_text_channel_->initiatorContact()); // HACK: We should not create a new Contact object here, we already have one.
	}

	void TextChatSessionRequest::OnTextChatSessionReady()
	{
		emit Ready(this);
	}

} // end of namespace: TpQt4Communication
