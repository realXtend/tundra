#ifndef incl_Communication_TelepathyIM_ConnectionProvider_h
#define incl_Communication_TelepathyIM_ConnectionProvider_h

#include "Foundation.h"
#include "..\interface.h"
#include "Connection.h"

namespace TelepathyIM
{
	/**
	 *  Offers IM functionaly in Opensim protocol (udp)
	 *
	 */
	class ConnectionProvider : Communication::ConnectionProviderInterface
	{
	public:
		//! 
		ConnectionProvider(Foundation::Framework* framework);

		virtual QStringList GetSupportedProtocols() const;

		//! Required fields: all
		virtual Communication::ConnectionPtr OpenConnection(const Communication::CredentialsInterface& credentials);

		//! 
		virtual Communication::ConnectionVector GetConnections() const;
	private:
		Communication::ConnectionVector connections_;
		Foundation::Framework* framework_;
	};

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_ConnectionProvider_h
