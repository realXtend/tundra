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
	class TPMessage: public MessageInterface
	{
		friend class TelepathyCommunication;
	public:
		TPMessage();
		TPMessage(ParticipantPtr author);
		Communication::ParticipantPtr GetAuthor();
		std::string GetTimeStamp();
	protected:
		std::string GetSessionId(); // called by TelepathyCommunication

		ParticipantPtr author_;
	};

	/*
	 *
	 */
	class TPIMMessage: public TPMessage, public IMMessageInterface
	{
		friend class TelepathyCommunication;
		friend class TPIMSession;
	public:
		TPIMMessage();
		TPIMMessage(ParticipantPtr author, std::string text); // should be private ?
		TPIMMessage(std::string text); // should be private ?
		void SetSession(SessionPtr s); // SessionInterface::CreateIMMessage(text), might be a better choice
		void SetText(std::string t);
		std::string GetText();

		// TODO: Fix this - these are TPMessage class methods...
		virtual std::string GetTimeStamp();
		virtual ParticipantPtr GetAuthor();

	protected:
		std::string text_;
		SessionPtr session_;
//		ParticipantPtr author_;
	};

} // end of namespace: Communication

#endif // incl_TPIMMessage_h