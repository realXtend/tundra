#include "Connection.h"
#include "StableHeaders.h"
//#include "RexLogicModule.h" // chat

namespace OpensimIM
{

	Connection::Connection(Foundation::Framework* framework): framework_(framework), name_(""), protocol_(OPENSIM_IM_PROTOCOL), server_(""), reason_("")
	{
		RequestFriendlist();
		RegisterConsoleCommands();
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
		//! \todo IMPLEMENT
		return Communication::ChatSessionPtr();
	}

	Communication::ChatSessionPtr Connection::OpenChatSession(const QString &channel)
	{
		//! \todo check if chat session on given channel already exist
		//!       and if so then return that session object

		ChatSession* session = new ChatSession(framework_);
		public_chat_sessions_.push_back(session);
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

	void Connection::RegisterConsoleCommands()
	{
		boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
        if ( !console_service )
        {
			LogError("Cannot register console commands :command service not found.");
			return;
		}
	}

	void Connection::RequestFriendlist()
	{
		//! \todo async xmplrpc "get_user_friend_list"(owenerID)
	}

} // end of namespace: OpensimIM
