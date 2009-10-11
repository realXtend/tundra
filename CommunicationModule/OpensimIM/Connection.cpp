#include "Connection.h"
#include "StableHeaders.h"
//#include "RexLogicModule.h" // chat

namespace OpensimIM
{

	Connection::Connection(Foundation::Framework* framework): framework_(framework), name_(""), protocol_(OPENSIM_IM_PROTOCOL), server_(""), reason_("")
	{

	}

	Connection::~Connection()
	{

	}

	
	QString Connection::GetName() const
	{
		return name_;
	}

	
	QString Connection::GetProtocol() const
	{
		return protocol_;
	}

	Communication::ConnectionInterface::State Connection::GetState() const
	{
		return state_;
	}

	QString Connection::GetServer() const
	{
		return server_;
	}

	QString Connection::GetReason() const
	{
		return reason_;
	}

	Communication::ContactGroupInterface* Connection::GetContacts() const
	{
		//! \todo IMPELEMENT
		return NULL;
	}

		
	QStringList Connection::GetAvailablePresenceStatusOptions() const
	{
		//! \todo IMPLEMENT
		return QStringList();
	}

	Communication::ChatSessionPtr Connection::OpenChatSession(const Communication::ContactInterface &contact)
	{
		ChatSession* session = new ChatSession(framework_);
		chat_sessions_.push_back(session);
		return Communication::ChatSessionPtr(session);
	}
	
	void Connection::SendFriendRequest(const QString &target, const QString &message)
	{
		//! \todo IMPLEMENT
	}

	Communication::FriendRequestVector Connection::GetFriendRequests() const
	{
		//! \todo IMPLEMENT
		return Communication::FriendRequestVector();
	}

		
	void Connection::Close()
	{
		state_ = STATE_CLOSED;
		emit ConnectionClosed( *this );
	}

} // end of namespace: OpensimIM
