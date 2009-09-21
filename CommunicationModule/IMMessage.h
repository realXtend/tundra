#ifndef incl_Comm_IMMessage_h
#define incl_Comm_IMMessage_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include <QTime>
#include "Communication.h"

namespace TpQt4Communication
{
	class Message
	{
	public:
		Message(std::string text);
		std::string GetText();
		Address GetAuthor();
		int GetTimeStamp();
	private:
		std::string text_;
		QTime time_stamp_; 
		Address author_;
	};
	typedef std::vector<Message*> MessageVector;
	

} // end of namespace: TpQt4Communication

#endif // incl_IMMessage_h
