#include "StableHeaders.h"
#include "Foundation.h"

#include "TPIMMessage.h"


namespace Communication
{

	TPMessage::TPMessage(): time_stamp_(CreateTimeStamp())
	{
	}

    /**
	 * @return timestamp in format HH:MM
	 **/
	std::string TPMessage::CreateTimeStamp()
	{
		time_t t;
		struct tm* time_info;
		time(&t);
		time_info = localtime(&t);
		int hours = time_info->tm_hour;
		int minutes = time_info->tm_min;

		std::stringstream stream;
		if (hours < 10)
			stream << "0";
		stream << hours;
		stream << ":";
		if (minutes < 10)
			stream << "0";
		stream << minutes;
		return stream.str();
	}

	TPMessage::TPMessage(ParticipantPtr author): time_stamp_(CreateTimeStamp()), author_(author)
	{
	}

	ParticipantPtr TPMessage::GetAuthor()
	{
		return author_;
	}

	std::string TPMessage::GetTimeStamp()
	{
		return time_stamp_;
	}


	//
	// TPIMMessage ----------------------------------->
	// 


	/**
	 * Default constructor
	 **/
	TPIMMessage::TPIMMessage()
	{
		TPMessage::TPMessage();
	}

	/**
	 * @param author The author participant of this message
	 * @param text Text contect of this message
	 **/
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
		return time_stamp_;
	}

	ParticipantPtr TPIMMessage::GetAuthor()
	{
		return author_;
	}

}
