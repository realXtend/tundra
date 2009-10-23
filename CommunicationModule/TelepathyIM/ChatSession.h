#ifndef incl_Communication_TelepathyIM_ChatSession_h
#define incl_Communication_TelepathyIM_ChatSession_h

#include <QStringList>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
#include "Foundation.h"
#include "..\interface.h"
#include "ContactGroup.h"
#include "ChatMessage.h"

namespace TelepathyIM
{
	/**
	 *  Text message based communication session with one or more participants.
	 *  This can represents irc channel or jabber conversation.
	 * 
	 *  This uses Telepathy text channel to communicate with IM server.
	 */
	class ChatSession : public Communication::ChatSessionInterface
	{
		Q_OBJECT
	public:
		ChatSession(Tp::TextChannelPtr tp_text_channel);		
//		ChatSession(Tp::Connection* tp_connection);

		virtual ~ChatSession();

		//! Send a text message to chat session
		//! @param text The message
		virtual void SendMessage(const QString &text);

		//! @return State of the session
		virtual Communication::ChatSessionInterface::State GetState() const;

		//! Closes the chat session. No more messages can be send or received. 
		//! Causes Closed signals to be emitted.
		virtual void Close();

		//! @return all known participants of the chat session
		virtual Communication::ChatSessionParticipantVector GetParticipants() const;

		//! @return the message history of this chat sessions
		virtual Communication::ChatMessageVector GetMessageHistory();

	protected:
		State state_;
		Tp::TextChannelPtr tp_text_channel_;
		Tp::Connection* tp_conneciton_;
		QStringList send_buffer_;
		ChatMessageVector message_history_;
		ChatSessionParticipantVector participants_;
	protected slots:
		//! This method is called ONLY when session is established by client 
		//! and it's NOT called when the session is established by server
		virtual void OnTextChannelCreated(Tp::PendingOperation* op);
		virtual void OnTextChannelReady(Tp::PendingOperation* op);
		virtual void ChatSession::OnMessageSendAck(Tp::PendingOperation* op);
		virtual void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &); // todo


	};
	
} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_ChatSession_h
