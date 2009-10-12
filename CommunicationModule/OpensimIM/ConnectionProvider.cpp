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

	Communication::ConnectionPtr ConnectionProvider::OpenConnection(const Communication::CredentialsInterface& credentials)
	{
		Connection* c = new Connection(framework_);
		connections_.push_back(c);
		return Communication::ConnectionPtr(c);
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
		Communication::ConnectionPtr conn;
		try
		{
			Communication::Credentials credentials("", "", "", 0); 
			conn = OpenConnection(credentials);
			
			QString channel = "0";
			Communication::ChatSessionPtr chat = conn->OpenChatSession(channel);

			QString message = "Hello world!";
			chat->SendMessage(message);
		}
		catch(std::exception& e)
		{
			QString ret = "OpensimIM test failed: ";
			ret.append( e.what() );
			return Console::ResultFailure( ret.toStdString() );
		}
		if ( conn->GetState() == Communication::ConnectionInterface::STATE_READY )
			conn->Close();

		QString ret = "";
		return Console::ResultSuccess( ret.toStdString() );
	}

} // end of namespace: OpensimIM
