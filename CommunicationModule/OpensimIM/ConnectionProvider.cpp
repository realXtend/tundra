#include <QStringList>
#include "ConnectionProvider.h"
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
		Connection* connection = new Connection(framework_, credentials.GetUserID());
		connections_.push_back(connection);
		connect(connection, SIGNAL( ConnectionReady(const Communication::ConnectionInterface&) ), SLOT( OnConnectionReady(const Communication::ConnectionInterface&) ));
		connect(connection, SIGNAL( FriendRequestReceived(Communication::FriendRequestInterface&) ), SLOT( OnFriendRequestReceived(Communication::FriendRequestInterface&) ));
		if ( connection->GetState() == Communication::ConnectionInterface::STATE_OPEN )
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
			vector.push_back(*i);
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

	static Communication::ChatSessionInterface* session = NULL;
	Console::CommandResult ConnectionProvider::OnConsoleCommandTest(const Core::StringVector &params)
	{
		try
		{
			assert( connections_.size() == 1 );
			QString user_id = "8bfe35a2-56a6-49fd-b0ad-c7da0aab5bf9";
			
			if (session == NULL)
				session = connections_[0]->OpenPrivateChatSession(user_id);
			session->SendMessage("Hello there!");
			session->SendMessage("How are you");
			LogInfo("Chat history:");
			Communication::ChatMessageVector history = session->GetMessageHistory();
			for (Communication::ChatMessageVector::iterator i = history.begin(); i != history.end(); ++i)
			{
				QString from = (*i)->GetOriginator()->GetName();
				QString text = (*i)->GetText();
				QString time = (*i)->GetTimeStamp().toString();
				QString note = QString(" ").append(time).append(" - ").append(from).append(": ").append(" ").append(text);
				LogInfo(note.toStdString());
			}
			//session->Close();
		}
		catch(std::exception& e)
		{
			QString ret = "OpensimIM test failed: ";
			ret.append( e.what() );
			return Console::ResultFailure( ret.toStdString() );
		}

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
			//! @todo agent_id to credentials 
			Communication::Credentials credentials(OPENSIM_IM_PROTOCOL, "", "", "", 0); 
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
		// Print friend list in debug mode
		Communication::ContactGroupInterface& friend_list = connection.GetContacts();
		LogDebug(friend_list.GetName().toStdString());

		Communication::ContactVector contacts = friend_list.GetContacts();
		for (Communication::ContactVector::iterator i = contacts.begin(); i != contacts.end(); ++i)
		{
			QString message = (*i)->GetName().append("(").append((*i)->GetID()).append(")");
			LogDebug(message.toStdString());
		}
		emit( ConnectionOpened(&connection) );
	}

	void ConnectionProvider::OnFriendRequestReceived(Communication::FriendRequestInterface& request)
	{
		QString message = "Friend request from ";
		message.append(request.GetOriginatorName());
		LogInfo(message.toStdString());
	}

} // end of namespace: OpensimIM
