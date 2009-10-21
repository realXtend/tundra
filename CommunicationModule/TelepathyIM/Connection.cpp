#include "Connection.h"

namespace TelepathyIM
{
	Connection::Connection(Tp::ConnectionManagerPtr tp_connection_manager, const Communication::CredentialsInterface &credentials) : tp_connection_manager_(tp_connection_manager), name_("Gabble"), protocol_("jabber"), state_(STATE_INITIALIZING)
	{
		CreateTpConnection(credentials);
	}

	void Connection::CreateTpConnection(const Communication::CredentialsInterface &credentials)
	{
		QVariantMap params;

		params.insert("account", credentials.GetUserID());
		params.insert("password", credentials.GetPassword());
		params.insert("server", credentials.GetServer());
		params.insert("port", QVariant( (unsigned int)credentials.GetPort() ));
		
		//std::string message = "Try to open connection to IM server: ";
		//message.append( server.toStdString () );
		//LogInfo(message);
		Tp::PendingConnection *pending_connection = tp_connection_manager_->requestConnection(credentials.GetProtocol(), params);
		QObject::connect(pending_connection, SIGNAL( finished(Tp::PendingOperation *) ), SLOT( OnConnectionCreated(Tp::PendingOperation *) ));
		server_ = credentials.GetServer();
	}

	Connection::~Connection()
	{
		if (!tp_connection_.isNull())
			tp_connection_->requestDisconnect();
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

	Communication::ContactGroupInterface& Connection::GetContacts()
	{
		return friend_list_;
	}

	QStringList Connection::GetAvailablePresenceStatusOptions() const
	{
		//! @todo IMPLEMENT
		QStringList options;
		return options;
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const Communication::ContactInterface &contact)
	{
		//! @todo IMPLEMENT
		return NULL;
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const QString& user_id)
	{
		//! @todo IMPLEMENT
		return NULL;
	}

	Communication::ChatSessionInterface* Connection::OpenChatSession(const QString &channel)
	{
		//! @todo IMPLEMENT
		return NULL;
	}

	void Connection::SendFriendRequest(const QString &target, const QString &message)
	{
		//! @todo IMPLEMENT
	}

	Communication::FriendRequestVector Connection::GetFriendRequests() const
	{
		//! @todo IMPLEMENT
		Communication::FriendRequestVector requests;
		return requests;
	}

	void Connection::Close()
	{
		if ( tp_connection_.isNull() )
			return; // nothing to close

		Tp::PendingOperation* op = tp_connection_->requestDisconnect();
		connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnConnectionClosed(Tp::PendingOperation*) ));
	}

	void Connection::OnConnectionCreated(Tp::PendingOperation *op)
	{
		state_ = STATE_OPEN;
		emit( ConnectionReady(*this) );
	}

	void Connection::OnConnectionClosed(Tp::PendingOperation *op)
	{
		state_ = STATE_CLOSED;
		emit( ConnectionClosed(*this) );
	}

} // end of namespace: TelepathyIM
