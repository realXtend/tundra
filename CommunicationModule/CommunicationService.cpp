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

        event_category_opensimnetworkin_ = framework_->GetEventManager()->QueryEventCategory("OpenSimNetworkIn");
        event_category_networkstate_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
	}

	CommunicationService::~CommunicationService(void)
	{
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
		return CommunicationService::instance_;
	}

	void CommunicationService::RegisterConnectionProvider( ConnectionProviderInterface* const provider )
	{
		connection_providers_.push_back(provider);
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

	Communication::ConnectionPtr CommunicationService::OpenConnection(const CredentialsInterface &credentials)
	{
		QString protocol = credentials.GetProtocol();
		ConnectionVector connections = GetConnections(protocol);
		if (connections.size() == 0)
		{
			throw Core::Exception("Cannot open connection, protocol is not supported");
		}
		return connections[0]; // We return first connection which support the protocol
	}
	
	ConnectionVector CommunicationService::GetConnections() const
	{
		return connections_;
	}

	ConnectionVector CommunicationService::GetConnections(const QString &protocol) const
	{
		return connections_per_protocol_.value(protocol);
	}

	bool CommunicationService::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
	{
		for (ConnectionProviderVector::iterator i = connection_providers_.begin(); i != connection_providers_.end(); ++i)
		{
			NetworkEventHandlerInterface* handler = dynamic_cast<NetworkEventHandlerInterface*>( *i );
			if ( handler )
			{
		        if ( category_id == event_category_opensimnetworkin_ )
				{
					bool ret = handler->HandleNetworkEvent( data );
					if ( ret )
						return true;
				}
		        if ( category_id == event_category_networkstate_ )
				{
					bool ret = handler->HandleNetworkStateEvent( data );
					if ( ret )
						return true;
				}
			}
			return false;
		}
	}

} // end of namespace: Communication
