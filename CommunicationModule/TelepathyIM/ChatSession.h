#ifndef incl_Communication_TelepathyIM_ChatSession_h
#define incl_Communication_TelepathyIM_ChatSession_h

#include <QStringList>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/PendingOperation>
#include "Foundation.h"
#include "..\interface.h"
#include "ContactGroup.h"

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
		ChatSession(Tp::Connection* tp_connection);

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
		Tp::Connection* tp_conneciton_;
	};
	
} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_ChatSession_h
