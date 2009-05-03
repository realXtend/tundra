
#include "StableHeaders.h"
#include "Foundation.h"

#include "TPIMMessage.h"


namespace Communication
{

	TPMessage::TPMessage(int session_id): session_id_(session_id)
	{

	}

ParticipiantPtr TPMessage::GetAuthor()
{
	return author_;
}

std::string TPMessage::GetTimeStamp()
{
	return "00:00"; // todo: set real time at constructor
}

int TPMessage::GetSessionId()
{
	return session_id_;
}


TPIMMessage::TPIMMessage(int session_id): TPMessage(session_id)
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