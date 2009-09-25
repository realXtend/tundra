#ifndef incl_Comm_IMMessage_h
#define incl_Comm_IMMessage_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include <QTime>
#include "Communication.h"
#include "Contact.h"

namespace TpQt4Communication
{
	class ChatSession;

	/**
     *
	 *
	 *
	 */
	class ChatMessage
	{
		friend class ChatSession;
		ChatMessage(std::string text, Contact* author);
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
	typedef std::vector<ChatMessage*> ChatMessageVector;
	

} // end of namespace: TpQt4Communication

#endif // incl_IMMessage_h
