#include "ConnectionProvider.h"
#include <QStringList>
#include "Connection.h"
#include "../Credentials.h"

namespace OpensimIM
{

	ConnectionProvider::ConnectionProvider(Foundation::Framework* framework) : framework_(framework)
	{
		RegisterConsoleCommands();	
	}

	ConnectionProvider::~ConnectionProvider()
	{
		CloseConnections();
		for ( ConnectionVector::iterator i = connections_.begin(); i != connections_.end(); ++i)
		{
			delete *i;
			*i = NULL;
		}
		connections_.clear();
	}

	QStringList ConnectionProvider::GetSupportedProtocols() const
	{
		QStringList protocols;
		protocols.append(OPENSIM_IM_PROTOCOL);
		return protocols;
	}

	Communication::ConnectionInterface* ConnectionProvider::OpenConnection(const Communication::CredentialsInterface& credentials)
	{
		Connection* connection = new Connection(framework_);
		connections_.push_back(connection);
		connect(connection, SIGNAL( ConnectionReady(const Communication::ConnectionInterface&) ), SLOT( OnConnectionReady(const Communication::ConnectionInterface&) ));
		connect(connection, SIGNAL( FriendRequestReceived(const Communication::FriendRequestInterface&) ), SLOT( OnFriendRequestReceived(const Communication::FriendRequestInterface&) ));
		if ( connection->GetState() == Communication::ConnectionInterface::STATE_READY )
		{
			OnConnectionReady(*connection);
		}

		return connection;
	}

	Communication::ConnectionVector ConnectionProvider::GetConnections() const
	{
		Communication::ConnectionVector vector;

		for ( ConnectionVector::const_iterator i = connections_.begin(); i != connections_.end(); ++i)
		{
			vector.push_back(Communication::ConnectionPtr(*i));
		}
		return vector;
	}

	void ConnectionProvider::RegisterConsoleCommands()
	{
		boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
        if ( !console_service )
        {
			LogError("Cannot register console commands :command service not found.");
			return;
		}

		console_service->RegisterCommand(Console::CreateCommand("opensimim test", "Test IM connestion by sending a text message to public chat", Console::Bind(this, &ConnectionProvider::OnConsoleCommandTest)));
	}

	Console::CommandResult ConnectionProvider::OnConsoleCommandTest(const Core::StringVector &params)
	{
		Communication::ConnectionInterface* conn;
		try
		{
			Communication::Credentials credentials("", "", "", 0); 
			conn = OpenConnection(credentials);
			
			QString channel = "0";
			Communication::ChatSessionInterface* chat = conn->OpenChatSession(channel);

			QString message = "Hello world!";
			chat->SendMessage(message);
		}
		catch(std::exception& e)
		{
			QString ret = "OpensimIM test failed: ";
			ret.append( e.what() );
			return Console::ResultFailure( ret.toStdString() );
		}
//		if ( conn->GetState() == Communication::ConnectionInterface::STATE_READY )
//			conn->Close();

		QString ret = "";
		return Console::ResultSuccess( ret.toStdString() );
	}

	bool ConnectionProvider::HandleNetworkEvent(Foundation::EventDataInterface* data)
	{
		for (ConnectionVector::iterator i = connections_.begin(); i != connections_.end(); ++i)
		{
			if ( (*i)->HandleNetworkEvent(data) )
				return true;
		}
		return false;
	}

	void ConnectionProvider::CloseConnections()
	{
		for (ConnectionVector::iterator i = connections_.begin(); i != connections_.end(); ++i)
		{
			(*i)->Close();
		}
	}

	bool ConnectionProvider::HandleNetworkStateEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
	{
        if (event_id == OpenSimProtocol::Events::EVENT_SERVER_CONNECTED)
		{
			Communication::Credentials credentials(OPENSIM_IM_PROTOCOL, "", "", 0); 
			Communication::ConnectionInterface* conn = OpenConnection(credentials);
		}

		if (event_id == OpenSimProtocol::Events::EVENT_SERVER_DISCONNECTED || event_id == OpenSimProtocol::Events::EVENT_CONNECTION_FAILED)
		{
			CloseConnections();
		}

		return false;
	}

	void ConnectionProvider::OnConnectionReady(Communication::ConnectionInterface& connection)
	{
		emit( ConnectionOpened(connection) );
	}

	void ConnectionProvider::OnFriendRequestReceived(const Communication::FriendRequestInterface& request)
	{
		QString message = "Friend request from ";
		message.append(request.GetOriginatorName());
		LogInfo(message.toStdString());
	}

} // end of namespace: OpensimIM
