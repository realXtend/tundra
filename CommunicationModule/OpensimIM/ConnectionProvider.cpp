#include "ConnectionProvider.h"
#include <QStringList>
#include "Connection.h"

namespace OpensimIM
{

	ConnectionProvider::ConnectionProvider(Foundation::Framework* framework) : framework_(framework)
	{
		
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

} // end of namespace: OpensimIM
