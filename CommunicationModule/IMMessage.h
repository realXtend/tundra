#ifndef incl_Comm_IMMessage_h
#define incl_Comm_IMMessage_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include <QTime>
#include "Communication.h"
#include "Contact.h"

namespace TpQt4Communication
{
	class TextChatSession;

	/**
     *
	 *
	 *
	 */
	//! todo: rename to ChatMessage
	class Message
	{
		friend class TextChatSession;
		Message(std::string text, Contact* author);
	public:
		std::string GetText();

		//! Return originator of this chat message
		Contact* GetAuthor();
		QTime GetTimeStamp();
	private:
		std::string text_;
		QTime time_stamp_; 
		Contact* author_;
	};
	typedef std::vector<Message*> MessageVector;
	

} // end of namespace: TpQt4Communication

#endif // incl_IMMessage_h
