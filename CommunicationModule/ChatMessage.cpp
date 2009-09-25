#include "StableHeaders.h"
#include "Foundation.h"

#include "ChatMessage.h"


namespace TpQt4Communication
{
	ChatMessage::ChatMessage(std::string text, Contact* author) : text_(text), author_(author), time_stamp_(QTime::currentTime())
	{
	}

	std::string ChatMessage::GetText()
	{
		return text_;
	}

	Contact* ChatMessage::GetAuthor()
	{
		return author_;
	}

	QTime ChatMessage::GetTimeStamp()
	{
		return time_stamp_;
	}


} // end of namespace: TpQt4Communication
