#ifndef incl_IMMessage_h
#define incl_IMMessage_h

#include "Foundation.h"
#include "EventDataInterface.h"



namespace Communication
{
	/**
	 *  Implementation of MessageInterface
	 *  @todo: implement author
	 */
	class Message: public MessageInterface
	{
		friend class CommunicationManager;
	public:
		Message();
		Message(ParticipantPtr author);
		virtual Communication::ParticipantPtr GetAuthor();
		virtual std::string GetTimeStamp();
	protected:
		virtual std::string Message::CreateTimeStamp();

		std::string time_stamp_;
		ParticipantPtr author_;
		SessionPtr session_;
	};

	/**
	 *  Implementation of IMMessageInterface
	 *
	 *  @todo solve the inheritance issue 
	 */
	class IMMessage: public Message, public IMMessageInterface
	{
		friend class CommunicationManager;
		friend class IMSession;
	public:
		IMMessage();
		IMMessage(ParticipantPtr author, std::string text); // should be private ?
		IMMessage(std::string text); // should be private ?
		void SetSession(SessionPtr s); // SessionInterface::CreateIMMessage(text), might be a better choice
		void SetText(std::string t);
		std::string GetText();

		//! @TODO: Fix this - these are Message class methods...
		virtual std::string GetTimeStamp();
		virtual ParticipantPtr GetAuthor();

	protected:
		std::string text_;
	};

} // end of namespace: Communication

#endif // incl_IMMessage_h