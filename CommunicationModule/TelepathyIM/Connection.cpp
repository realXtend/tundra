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
	    if (op->isError())
		{
			//std::string message = "Cannot create a connection object: ";
			//message.append( op->errorMessage().toStdString() );
			//LogError(message);
			state_ = STATE_ERROR;
			throw Core::Exception( op->errorMessage().toStdString().c_str() );
		}
		
		Tp::PendingConnection *c = qobject_cast<Tp::PendingConnection *>(op);
		tp_connection_ = c->connection();

		//std::string message = "Connection created to IM server.";
		//LogInfo(message);

		QObject::connect(tp_connection_->requestConnect(),
					     SIGNAL(finished(Tp::PendingOperation *)),
						 SLOT(OnConnectionConnected(Tp::PendingOperation *)));

		QObject::connect(tp_connection_.data(),
			             SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
						 SLOT(OnConnectionInvalidated(Tp::DBusProxy *, const QString &, const QString &)));


		state_ = STATE_OPEN;
		emit( ConnectionReady(*this) );
	}

	void Connection::OnConnectionConnected(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			//QString reason = "Cannot connect to IM server:: ";
			//reason.append(op->errorMessage());
			//LogError(reason.toStdString());
			state_ = STATE_ERROR;
			throw Core::Exception( op->errorMessage().toStdString().c_str() );
		}
		//std::string message = "Connection established successfully to IM server.";
		//LogInfo(message);

		Tp::Features features;
		features.insert(Tp::Connection::FeatureSimplePresence);
		features.insert(Tp::Connection::FeatureRoster);
		features.insert(Tp::Connection::FeatureSelfContact);
		features.insert(Tp::Connection::FeatureCore);
		QObject::connect(tp_connection_->becomeReady(features),
		                 SIGNAL(finished(Tp::PendingOperation *)),
						 SLOT(OnConnectionReady(Tp::PendingOperation *)));

		if( tp_connection_->interfaces().contains(TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS) )
		{
			QObject::connect(tp_connection_->requestsInterface(),
                SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
                SLOT(OnNewChannels(const Tp::ChannelDetailsList&)));
		}
	}

	void Connection::OnConnectionReady(Tp::PendingOperation *op)
	{
	    if (op->isError())
		{
			//QString message = "Connection initialization to IM server failed: ";
			//message.append(op->errorMessage());
			//LogError(message.toStdString());
			state_ = STATE_ERROR;
			throw Core::Exception( op->errorMessage().toStdString().c_str() );
		}
//		LogInfo("Connection to IM server ready.");

		//user_ = new User(tp_connection_);
		//HandleNewContacts();

		//QObject::connect(tp_connection_->contactManager(),
  //          SIGNAL(presencePublicationRequested(const Tp::Contacts &)),
  //          SLOT(OnPresencePublicationRequested(const Tp::Contacts &)));

		state_ = STATE_OPEN;
		emit( ConnectionReady(*this) );
	}

	void Connection::OnNewChannels(const Tp::ChannelDetailsList& details)
	{

	}

	void Connection::OnConnectionInvalidated(Tp::PendingOperation *op)
	{

	}

	void Connection::OnConnectionClosed(Tp::PendingOperation *op)
	{
		state_ = STATE_CLOSED;
		emit( ConnectionClosed(*this) );
	}

} // end of namespace: TelepathyIM
