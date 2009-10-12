#ifndef incl_Communication_OpensimIM_Provider_h
#define incl_Communication_OpensimIM_Provider_h

#include "Foundation.h"
#include "..\interface.h"
#include "Connection.h"

namespace OpensimIM
{
	/**
	 *  Offers IM functionaly in Opensim protocol (udp)
	 *
	 */
	class ConnectionProvider : public  Communication::ConnectionProviderInterface
	{
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "Communication(OpensimIM)"; } // for logging functionality

	public:
		//! \param framework Framework object 
		ConnectionProvider(Foundation::Framework* framework);

		//! Provides list of supported protocols by this provider
		virtual QStringList GetSupportedProtocols() const;

		//! Open a new connection to IM server woth given credentials
		virtual Communication::ConnectionPtr OpenConnection(const Communication::CredentialsInterface& credentials);

		//! Provides all Connections objects created with this provider
		virtual Communication::ConnectionVector GetConnections() const;

	protected:
		//! commands: "opensimim test"
		virtual void RegisterConsoleCommands();

		//! Test IM connection vy creating connection, open text chat on public world chat and sent
		//! a text message
		virtual Console::CommandResult OnConsoleCommandTest(const Core::StringVector &params);

	private:
		ConnectionVector connections_;
//		Communication::ConnectionVector connections_;
		Foundation::Framework* framework_;
	};

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_Provider_h
