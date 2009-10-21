#ifndef incl_Communication_OpensimIM_Provider_h
#define incl_Communication_OpensimIM_Provider_h

#include "Foundation.h"
#include "..\interface.h"
#include "..\CommunicationService.h"
#include "Connection.h"

#define OPENSIM_IM_PROTOCOL "OpensimUDP"
#define MODREX_IM_PROTOCOL "ModRex"

namespace OpensimIM
{
	/**
     *  This class is only used by CommunicationService object. Do not use this
	 *  directly!
     *
	 *  Offers IM functionaly in Opensim protocol (udp)
	 *
	 */
	class ConnectionProvider : public  Communication::ConnectionProviderInterface, public Communication::NetworkEventHandlerInterface
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "Communication(OpensimIM)"; } // for logging functionality
		
	public:
		//! \param framework Framework object 
		ConnectionProvider(Foundation::Framework* framework);

		//!
		virtual ~ConnectionProvider();

		//! Provides list of supported protocols by this provider
		virtual QStringList GetSupportedProtocols() const;

		//! Open a new connection to IM server woth given credentials
		virtual Communication::ConnectionInterface* OpenConnection(const Communication::CredentialsInterface& credentials);

		//! Provides all Connections objects created with this provider
		virtual Communication::ConnectionVector GetConnections() const;

		//! Handle network event
		virtual bool HandleNetworkEvent(Foundation::EventDataInterface* data);

		//! Handle network event
		virtual bool HandleNetworkStateEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

	protected:
		//! commands: "opensimim test"
		virtual void RegisterConsoleCommands();

		//! Test IM connection vy creating connection, open text chat on public world chat and sent
		//! a text message
		virtual Console::CommandResult OnConsoleCommandTest(const Core::StringVector &params);

		virtual void CloseConnections();

	private:
		ConnectionVector connections_;
		Foundation::Framework* framework_;

	protected slots:
		//! Make log entry about friend request
		void OnFriendRequestReceived(Communication::FriendRequestInterface& request);
		void OnConnectionReady(Communication::ConnectionInterface& connection);
	};

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_Provider_h
