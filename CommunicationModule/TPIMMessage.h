#ifndef incl_TPIMMessage_h
#define incl_TPIMMessage_h

#include "Foundation.h"
#include "EventDataInterface.h"

// todo: implement author

namespace Communication
{
	/*
	 *
	 */
	class TPMessage: public Message
	{
		friend class TelepathyCommunication;
	public:
		TPMessage(std::string session_id);
		Communication::ParticipiantPtr GetAuthor();
		std::string GetTimeStamp();
	protected:
		std::string GetSessionId(); // called by TelepathyCommunication

		Communication::ParticipiantPtr author_;
		std::string session_id_;
	};

	/*
	 *
	 */
	class TPIMMessage: public TPMessage, public IMMessage
	{
		friend class TelepathyCommunication;
		friend class TPIMSession;
	public:
		TPIMMessage(std::string session_id);
		void SetText(std::string t);
		std::string GetText();
	protected:
		std::string text_;
	};

} // end of namespace: Communication

#endif // incl_TPIMMessage_h