#include "CommunicationService.h"
#include <QList>
#include "NetworkEvents.h"

namespace Communication
{
	CommunicationService* CommunicationService::instance_ =  NULL; // static member function initialization

	CommunicationService::CommunicationService(Foundation::Framework* framework) : framework_(framework)
	{
		if (framework_ == NULL)
			throw Core::Exception("Cannot create communication service object, framework pointer is missing."); 

        event_category_networkinin_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
        event_category_networkstate_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
	}

	CommunicationService::~CommunicationService(void)
	{
		for (ConnectionProviderVector::iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
		{
			delete *i;
			*i = NULL;
		}
		connection_providers_.clear();
	}

	// static
	void CommunicationService::CreateInstance(Foundation::Framework* framework)
	{
		if (CommunicationService::instance_ == NULL)
			CommunicationService::instance_ = new CommunicationService(framework);
	}

	//! static
	CommunicationServiceInterface* CommunicationService::GetInstance()
	{
		if (CommunicationService::instance_ == 0)
			throw Core::Exception("CommunicationService is not initialized properly.");
		return CommunicationService::instance_;
	}

	void CommunicationService::RegisterConnectionProvider( ConnectionProviderInterface* const provider )
	{
		connection_providers_.push_back(provider);
		connect( provider, SIGNAL( ProtocolListUpdated(const QStringList&) ), SLOT( OnProtocolListUpdated(const QStringList&) ));
		connect( provider, SIGNAL( ConnectionOpened(Communication::ConnectionInterface*) ), SLOT( OnConnectionOpened(Communication::ConnectionInterface*) ));
		connect( provider, SIGNAL( ConnectionClosed(Communication::ConnectionInterface*) ), SLOT( OnConnectionClosed(Communication::ConnectionInterface*) ));
	}

	QStringList CommunicationService::GetSupportedProtocols() const
	{
		QStringList all_protocols;
		for (ConnectionProviderVector::const_iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
		{
			QStringList protocols = (*i)->GetSupportedProtocols();
			for (QStringList::const_iterator ii = protocols.begin(); ii != protocols.end(); ++ii)
			{
				QString protocol = (*ii);
				if ( !all_protocols.contains(protocol) )
					all_protocols.append(protocol);
			}
		}
		return all_protocols;
	}

	Communication::ConnectionInterface* CommunicationService::OpenConnection(const CredentialsInterface &credentials)
	{
		QString protocol = credentials.GetProtocol();
		//! @todo Check if connection already exist with given credentials 
		ConnectionProviderVector providers = GetConnectionProviders(protocol);
		if (providers.size() == 0)
		{
			throw Core::Exception("Cannot open connection, protocol is not supported");
		}
		Communication::ConnectionInterface* connection = providers[0]->OpenConnection(credentials);
		return connection;
	}
	
	ConnectionVector CommunicationService::GetConnections() const
	{
		return connections_;
	}

	ConnectionVector CommunicationService::GetConnections(const QString &protocol) const
	{
		ConnectionVector connections;
		for (ConnectionVector::const_iterator i = connections_.begin(); i != connections_.end(); ++i)
		{
			if ( (*i)->GetProtocol().compare( protocol ) == 0 )
				connections.push_back( *i );
		}
		return connections;
	}

	ConnectionProviderVector CommunicationService::GetConnectionProviders(const QString &protocol) const
	{
		ConnectionProviderVector providers;
		for (ConnectionProviderVector::const_iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
		{
			QStringList protocols = (*i)->GetSupportedProtocols();
			if ( protocols.contains(protocol) )
				providers.push_back(*i);
		}
		return providers;
	}

	bool CommunicationService::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
	{
		for (ConnectionProviderVector::iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
		{
			NetworkEventHandlerInterface* handler = dynamic_cast<NetworkEventHandlerInterface*>( *i );
			if ( handler )
			{
		        if ( category_id == event_category_networkinin_ )
				{
					bool ret = handler->HandleNetworkEvent( data );
					if ( ret )
						return true;
				}
		        if ( category_id == event_category_networkstate_ )
				{
					bool ret = handler->HandleNetworkStateEvent( event_id, data );
					if ( ret )
						return true;
				}
			}
			return false;
		}
		return false;
	}

	void CommunicationService::OnProtocolListUpdated(const QStringList& protocols)
	{
		for (QStringList::const_iterator i = protocols.begin(); i != protocols.end(); ++i)
		{
			if ( !supported_protocols_.contains(*i) )
				supported_protocols_.append( *i );
		}
	}

	void CommunicationService::OnConnectionOpened(Communication::ConnectionInterface* connection)
	{
		//! @todo connect to vital singals of the connection
		connections_.push_back(connection);
		emit( ConnectionOpened(connection) );

		connect(connection, SIGNAL( FriendRequestReceived(const Communication::FriendRequestInterface&) ), SLOT(OnFriendRequestReceived(const Communication::FriendRequestInterface&); )); 
	}

	void CommunicationService::OnConnectionClosed(Communication::ConnectionInterface* connection)
	{
	}

	void CommunicationService::OnFriendRequestReceived(const Communication::FriendRequestInterface& request)
	{
		QString message = "Friend request from ";
		message.append( request.GetOriginatorName() );
		LogDebug(message.toStdString());
	}

} // end of namespace: Communication
