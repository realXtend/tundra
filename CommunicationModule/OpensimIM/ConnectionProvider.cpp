// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <QStringList>
#include "ConnectionProvider.h"
#include "Connection.h"
#include "Credentials.h"
#include "ConsoleCommandServiceInterface.h"
#include "NetworkEvents.h"
#include "ServiceManager.h"
#include "Framework.h"
#include "CoreDefines.h"

#include "MemoryLeakCheck.h"

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
            Connection* connection = *i;
            SAFE_DELETE(connection);
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
        connect(connection, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( OnConnectionReady(Communication::ConnectionInterface&) )); // Isn't in use currently
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
    }

    bool ConnectionProvider::HandleNetworkEvent(Foundation::EventDataInterface* data)
    {
        for (ConnectionVector::iterator i = connections_.begin(); i != connections_.end(); ++i)
        {
            Connection* connection = *i;
            if ( connection->HandleNetworkEvent(data) )
                return true;
        }
        return false;
    }

    void ConnectionProvider::CloseConnections()
    {
        for (ConnectionVector::iterator i = connections_.begin(); i != connections_.end(); ++i)
        {
            Connection* connection = *i;
            connection->Close();
        }
        connections_.clear();
    }

    bool ConnectionProvider::SupportProtocol(QString &protocol) const
    {
        return protocol.compare(OPENSIM_IM_PROTOCOL) == 0;
    }

    bool ConnectionProvider::HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
        {
            //! @todo agent_id to credentials 

            //! TODO: CHECK THIS
//            Communication::Credentials credentials(OPENSIM_IM_PROTOCOL, "", "", "", 0); 
//            Communication::ConnectionInterface* conn = OpenConnection(credentials);
        }

        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED || event_id == ProtocolUtilities::Events::EVENT_CONNECTION_FAILED)
        {
            CloseConnections();
        }

        return false;
    }

    void ConnectionProvider::OnConnectionReady(Communication::ConnectionInterface& connection)
    {
        emit( ConnectionOpened(&connection) );
    }

} // end of namespace: OpensimIM
