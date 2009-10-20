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
		ChatSession(Foundation::Framework* framework, const QString &channel_id);
		ChatSession(Foundation::Framework* framework);

		//! \todo support to other range options
		virtual void SendMessage(const QString &text);

		//! 
		virtual void Close();

		//! Report chat session object about text message whitch originator is a avatart.
		virtual void MessageFromAgent(const QString &avatar_id, const QString &name, const QString &text);

		//! Report chat session object about text message whitch origin is the world server 
		virtual void MessageFromServer(const QString &text);

		//! Report chat session object about text message whitch origin is the object
		virtual void MessageFromObject(const QString &object_id, const QString &text);

		//! Provides known participants of this chat session
		//! Only participant who have send at least one text messages are listed here
		virtual Communication::ChatSessionParticipantVector GetParticipants() const;

		//! Provides ID of chat session
		virtual QString GetID() const;

		//! Return true if this chat session is private IM chat
		//! Return false if this is global chat
		virtual bool IsPrivateIMSession();
	protected:

		void SendPrivateIMMessage(const QString &text);
		void SendPublicChatMessage(const QString &text);

		QString channel_id_;
		ChatSessionParticipant server_participant_;
		ChatSessionParticipant* FindParticipant(const QString &uuid);
		bool private_im_session_;
		

	private:
		Foundation::Framework* framework_;
		ChatSessionParticipantVector participants_;
	};
	typedef boost::shared_ptr<ChatSession> ChatSessionPtr;
	typedef std::vector<ChatSession*> ChatSessionVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_ChatSession_h
