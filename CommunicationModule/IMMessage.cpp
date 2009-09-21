#include "StableHeaders.h"
#include "Foundation.h"

#include "IMMessage.h"


namespace TpQt4Communication
{

	Message::Message(std::string text)
	{
		text_ = text;
		author_ = "";
		time_stamp_ = QTime::currentTime();
	}

} // end of namespace: TpQt4Communication
