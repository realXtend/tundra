#include "StableHeaders.h"
#include "Foundation.h"

#include "TPIMMessage.h"


namespace Communication
{

	TPMessage::TPMessage(std::string session_id): session_id_(session_id)
	{

	}

	ParticipiantPtr TPMessage::GetAuthor()
	{
		return author_;
	}

	std::string TPMessage::GetTimeStamp()
	{
		return "00:00"; // todo: set real time to variable at constructor
	}

	std::string TPMessage::GetSessionId()
	{
		return session_id_;
	}

	TPIMMessage::TPIMMessage(std::string session_id): TPMessage(session_id), text_("")
	{
		
	}

	void TPIMMessage::SetText(std::string text)
	{
		this->text_ = text;
	}

	std::string TPIMMessage::GetText()
	{
		return this->text_;
	}
}
