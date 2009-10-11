#include "CommunicationService.h"
#include <QList>

namespace Communication
{
	CommunicationService* CommunicationService::instance_ =  NULL; // static member function initialization

	CommunicationService::CommunicationService(void)
	{
	}

	CommunicationService::~CommunicationService(void)
	{
	}

	//! static
	CommunicationServiceInterface* CommunicationService::GetInstance()
	{
		if (CommunicationService::instance_ == NULL)
			CommunicationService::instance_ = new CommunicationService();

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

} // end of namespace: Communication
