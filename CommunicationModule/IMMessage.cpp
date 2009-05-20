#include "StableHeaders.h"
#include "Foundation.h"

#include "IMMessage.h"


namespace Communication
{

	Message::Message(): time_stamp_(CreateTimeStamp())
	{
		author_ = ParticipantPtr(); // we don't know yet
	}

    /**
	 * @return timestamp in format "HH:MM"
	 **/
	std::string Message::CreateTimeStamp()
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

	Message::Message(ParticipantPtr author): time_stamp_(CreateTimeStamp()), author_(author)
	{
	}

	ParticipantPtr Message::GetAuthor()
	{
		return author_;
	}

	std::string Message::GetTimeStamp()
	{
		return time_stamp_;
	}

	// todo: Move to own file
	// IMMessage ----------------------------------->

	/**
	 * Default constructor
	 **/
	IMMessage::IMMessage()
	{
		Message::Message();
	}

	/**
	 * @param author The author participant of this message
	 * @param text Text contect of this message
	 **/
	IMMessage::IMMessage(ParticipantPtr author, std::string text): Message(author), text_(text)
	{
	}

	IMMessage::IMMessage(std::string text): text_(text)
	{
	}
	
	void IMMessage::SetSession(SessionPtr s)
	{
		session_ = s;
	}

	void IMMessage::SetText(std::string text)
	{
		this->text_ = text;
	}

	std::string IMMessage::GetText()
	{
		return this->text_;
	}

	std::string IMMessage::GetTimeStamp()
	{
		return time_stamp_;
	}

	ParticipantPtr IMMessage::GetAuthor()
	{
		return author_;
	}
}
