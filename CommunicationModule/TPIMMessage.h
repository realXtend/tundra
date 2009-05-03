#ifndef incl_TPIMMessage_h
#define incl_TPIMMessage_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	// MessageImpl
	class TPMessage: public Message
	{
	public:
		TPMessage(int session_id);
		Communication::ParticipiantPtr GetAuthor();
		std::string GetTimeStamp();
		int GetSessionId();
	private:
		Communication::ParticipiantPtr author_;
		int session_id_;
	};

	class TPIMMessage: public TPMessage
	{
	public:
		TPIMMessage(int session_id);
		void SetText(std::string t);
		std::string GetText();
	private:
		std::string text_;
	};

	typedef boost::shared_ptr<TPIMMessage> TPIMMessagePtr;
	typedef std::vector<TPIMMessagePtr> TPIMMessageList;

} // end of namespace: Communication

#endif // incl_TPIMMessage_h