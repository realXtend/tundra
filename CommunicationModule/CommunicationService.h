#ifndef incl_Comm_CommunicationService_h
#define incl_Comm_CommunicationService_h

#include <StableHeaders.h>
#include <ModuleInterface.h> // for logger
#include <QMap>
#include <QStringList>
#include "interface.h"

namespace Communication
{

	/**
	 *
	 *
	 */
	class NetworkEventHandlerInterface
	{
	public:
		//! Handle network event
		virtual bool HandleNetworkEvent(Foundation::EventDataInterface* data) = 0;
		virtual bool HandleNetworkStateEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data) = 0;
	};

	/**
	 *  Implements CommunicationServiceInterface 
	 *
	 */
	class CommunicationService : public CommunicationServiceInterface
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
		//! returns name of this module. Needed for logging.
		static const std::string NameStatic() { return "Communication"; } // for logging functionality
	public:
		CommunicationService(Foundation::Framework* framework);
		~CommunicationService(void);
		static void CreateInstance(Foundation::Framework* framework);

		//! Static method to provide singleton CommunicationServiceInterface object 
		static CommunicationServiceInterface* GetInstance();

		//! Register a ConnectionProvider object to communication service
		//! Without any connection provider communication service cannot provide
		//! any communication protocols.
		virtual void RegisterConnectionProvider( ConnectionProviderInterface* const provider);

		//! Provides list of all currently supported protocols
		virtual QStringList GetSupportedProtocols() const;

		//! Create new Connection object accordingly given credentials
		virtual ConnectionInterface* OpenConnection(const CredentialsInterface &credentials);

		//! Return all Connection objects
		virtual ConnectionVector GetConnections() const;

		//! Provides Connection objects which supports given protocol
		virtual ConnectionVector GetConnections(const QString &protocol) const;

		//! Handle events
		virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);

	protected:
		ConnectionProviderVector GetConnectionProviders(const QString &protocol) const;

		Foundation::Framework* framework_;
		static CommunicationService* instance_;
		ConnectionProviderVector connection_providers_;
		ConnectionVector connections_;
		QMap<QString, ConnectionVector> connections_per_protocol_;
		QStringList supported_protocols_;

		//! category id for incoming messages
		Core::event_category_id_t event_category_networkinin_;
        Core::event_category_id_t event_category_networkstate_;
	protected slots:
		virtual void OnProtocolListUpdated(const QStringList& protocols);
		virtual void OnConnectionOpened(Communication::ConnectionInterface*);
		virtual void OnConnectionClosed(Communication::ConnectionInterface*);
		virtual void OnFriendRequestReceived(Communication::FriendRequestInterface& request);
	};
} // end of namespace: Communication

#endif // incl_Comm_CommunicationService_h
