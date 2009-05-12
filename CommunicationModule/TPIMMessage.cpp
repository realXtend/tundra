#include "StableHeaders.h"
#include "Foundation.h"

#include "TPIMMessage.h"


namespace Communication
{

	TPMessage::TPMessage()
	{

	}

	TPMessage::TPMessage(ParticipantPtr author): author_(author)
	{
		// TODO: get timestamp 
	}

	ParticipantPtr TPMessage::GetAuthor()
	{
		return author_;
	}

	std::string TPMessage::GetTimeStamp()
	{
		return "00:00"; // todo: set real time to variable at constructor
	}

	//std::string TPMessage::GetSessionId()
	//{
	//	return session_id_;
	//}


	//
	// TPIMMessage ----------------------------------->
	// 


	TPIMMessage::TPIMMessage()
	{

	}

	TPIMMessage::TPIMMessage(ParticipantPtr author, std::string text): TPMessage(author), text_(text)
	{
		
	}

	TPIMMessage::TPIMMessage(std::string text): text_(text)
	{
		
	}
	
	void TPIMMessage::SetSession(SessionPtr s)
	{
		session_ = s;
	}



	void TPIMMessage::SetText(std::string text)
	{
		this->text_ = text;
	}

	std::string TPIMMessage::GetText()
	{
		return this->text_;
	}

	std::string TPIMMessage::GetTimeStamp()
	{
		return "00:00"; // todo: set real time to variable at constructor
	}

	ParticipantPtr TPIMMessage::GetAuthor()
	{
		return author_;
	}

}
