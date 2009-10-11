#ifndef incl_Communication_OpensimIM_ChatSession_h
#define incl_Communication_OpensimIM_ChatSession_h

#include "Foundation.h"
#include "..\interface.h"


namespace OpensimIM
{
	/**
	 * Text message based communication session with one or more participants.
	 * This can represents irc channel or jabber conversation.
	 *
	 */
	class ChatSession : public Communication::ChatSessionInterface
	{
	public:
		ChatSession(Foundation::Framework* framework);

		//! \todo support to other range options
		virtual void SendMessage(const QString &text);

		//! 
		virtual void Close();
	private:
		Foundation::Framework* framework_;
	};
//	typedef boost::shared_ptr<ChatSession> ChatSessionPtr;
	typedef std::vector<ChatSession*> ChatSessionVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_ChatSession_h