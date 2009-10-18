#ifndef incl_Comm_CommunicationService_h
#define incl_Comm_CommunicationService_h

#include "StableHeaders.h"
#include "interface.h"
#include "Foundation.h"
#include "ModuleInterface.h" // for logger
#include <QMap>
#include <QStringList>

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
		virtual ConnectionPtr OpenConnection(const CredentialsInterface &credentials);

		//! Return all Connection objects
		virtual ConnectionVector GetConnections() const;

		//! Provides Connection objects which supports given protocol
		virtual ConnectionVector GetConnections(const QString &protocol) const;

		//! Handle events
		virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);

	protected:
		virtual void CommunicationService::OnNewConnection(Communication::ConnectionInterface*);

		static CommunicationService* instance_;
		ConnectionVector connections_;
		ConnectionProviderVector connection_providers_;
		QMap<QString, ConnectionVector> connections_per_protocol_;
		Foundation::Framework* framework_;

		//! category id for incoming messages
		Core::event_category_id_t event_category_opensimnetworkin_;
        Core::event_category_id_t event_category_networkstate_;
	protected slots:
		void OnFriendRequestReceived(const Communication::FriendRequestInterface& request);

	};
} // end of namespace: Communication

#endif // incl_Comm_CommunicationService_h
