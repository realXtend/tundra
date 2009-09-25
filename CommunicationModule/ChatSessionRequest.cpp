#include "ChatSessionRequest.h"

namespace TpQt4Communication
{
	ChatSessionRequest::ChatSessionRequest(Tp::TextChannelPtr tp_text_channel):  tp_text_channel_(tp_text_channel)
	{
		session_ = new ChatSession(tp_text_channel);
		Tp::Features features;
		features.insert(Tp::TextChannel::FeatureMessageQueue);
		features.insert(Tp::TextChannel::FeatureCore);
		features.insert(Tp::TextChannel::FeatureMessageCapabilities);
		Tp::PendingReady* p =  tp_text_channel->becomeReady(features);
		QObject::connect(p, SIGNAL(finished(Tp::PendingOperation*)), SLOT( OnChatSessionReady() ) );
	}

	ChatSessionPtr ChatSessionRequest::Accept()
	{
		LogInfo("Chat session accepted.");
		return ChatSessionPtr(session_);
	}

	void ChatSessionRequest::Reject()
	{
		LogInfo("Chat session rejected.");
		Tp::PendingOperation* p = tp_text_channel_->requestClose();
		QObject::connect(p,
			SIGNAL( finished(Tp::PendingOperation* ) ),
			SLOT( OnTextChannelClosed(Tp::PendingOperation* ) ));
	}

	Address ChatSessionRequest::GetOriginator()
	{
		//! todo: implement
		return "NOT IMPLEMENTED."; 
	}

	std::string ChatSessionRequest::GetMessage()
	{
		return message_;
	}

	void ChatSessionRequest::OnTextChannelClosed(Tp::PendingOperation* op)
	{
		if ( op->isError())
		{
			LogInfo("Cannot close text channel.");
		}
		LogInfo("Text channel closed.");
	}

	Contact* ChatSessionRequest::GetOriginatorContact()
	{
		LogInfo("ChatSessionRequest::GetOriginatorContact()");
		if ( !tp_text_channel_->isReady() )
		{
			LogError("tp_text_channel_ == NULL");
			return NULL;
		}
		return new Contact(tp_text_channel_->initiatorContact()); // HACK: We should not create a new Contact object here, we already have one.
	}

	void ChatSessionRequest::OnChatSessionReady()
	{
		emit Ready(this);
	}

} // end of namespace: TpQt4Communication
