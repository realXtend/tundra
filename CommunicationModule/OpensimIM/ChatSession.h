#ifndef incl_Communication_OpensimIM_ChatSession_h
#define incl_Communication_OpensimIM_ChatSession_h

#include "Foundation.h"
#include "NetworkEvents.h"
#include "..\interface.h"
#include "ChatSessionParticipant.h"


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
		ChatSession(Foundation::Framework* framework, const QString channel_id);

		//! \todo support to other range options
		virtual void SendMessage(const QString &text);

		//! 
		virtual void Close();

		virtual void MessageFromServer(const QString &from, const QString &text);

		//!
		virtual Communication::ChatSessionParticipantVector GetParticipants() const;

		//! Provides ID of chat session
		virtual QString GetID() const;
	protected:
		QString channel_id_;
		ChatSessionParticipant* FindParticipant(const QString &uuid);

	private:
		Foundation::Framework* framework_;
		ChatSessionParticipantVector participants_;

	};
	typedef boost::shared_ptr<ChatSession> ChatSessionPtr;
	typedef std::vector<ChatSession*> ChatSessionVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_ChatSession_h
