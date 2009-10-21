#include "ChatSession.h"

namespace TelepathyIM
{
	ChatSession::ChatSession(Tp::Connection* tp_connection) : tp_conneciton_(tp_connection), state_(STATE_CLOSED)
	{
		//! @todo IMPLEMENT
	}

	ChatSession::~ChatSession()
	{
		//! @todo IMPLEMENT
	}

	void ChatSession::SendMessage(const QString &text)
	{
		//! @todo IMPLEMENT
	}

	Communication::ChatSessionInterface::State ChatSession::GetState() const
	{
		return state_;
	}

	void ChatSession::Close()
	{
		//! @todo IMPLEMENT
	}

	Communication::ChatSessionParticipantVector ChatSession::GetParticipants() const
	{
		//! @todo IMPLEMENT
		Communication::ChatSessionParticipantVector empty_vector;
		return empty_vector;
	}

	Communication::ChatMessageVector ChatSession::GetMessageHistory()
	{
		//! @todo IMPLEMENT
		Communication::ChatMessageVector empty_vector;
		return empty_vector;
	}
	
} // end of namespace: TelepathyIM
