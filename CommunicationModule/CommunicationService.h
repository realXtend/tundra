#ifndef incl_Comm_CommunicationService_h
#define incl_Comm_CommunicationService_h

#include "StableHeaders.h"
#include "interface.h"
#include <QMap>
#include <QStringList>

namespace Communication
{

	/**
	 *  Implements CommunicationServiceInterface 
	 *
	 */
	class CommunicationService : public CommunicationServiceInterface
	{
	public:
		CommunicationService(void);
		~CommunicationService(void);

		//! Static method to provide singleton CommunicationServiceInterface object 
		static CommunicationServiceInterface* GetInstance();

		//! Register a ConnectionProvider object to communication service
		//! Without any connection provider communication service cannot provide
		//! any communication protocols.
		virtual void RegisterConnectionProvider( ConnectionProviderInterface* const provider);

		//! Provides list of all currently supported protocols
		virtual QStringList GetSupportedProtocols() const;


		//! Create new Connection object accordingly given credentials
		virtual ConnectionPtr OpenConnection(const CredentialsInterface &credentials);

		//! Return all Connection objects
		virtual ConnectionVector GetConnections() const;

		//! Provides Connection objects which supports given protocol
		virtual ConnectionVector GetConnections(const QString &protocol) const;

	protected:
		static CommunicationService* instance_;
		ConnectionVector connections_;
		ConnectionProviderVector connection_providers_;
		QMap<QString, ConnectionVector> connections_per_protocol_;
	};
} // end of namespace: Communication

#endif // incl_Comm_CommunicationService_h
