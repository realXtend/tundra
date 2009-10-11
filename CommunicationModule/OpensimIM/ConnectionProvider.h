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
	public:
		//! \param framework Framework object 
		ConnectionProvider(Foundation::Framework* framework);

		//! Provides list of supported protocols by this provider
		virtual QStringList GetSupportedProtocols() const;

		//! Open a new connection to IM server woth given credentials
		virtual Communication::ConnectionPtr OpenConnection(const Communication::CredentialsInterface& credentials);

		//! Provides all Connections objects created with this provider
		virtual Communication::ConnectionVector GetConnections() const;

	private:
		ConnectionVector connections_;
//		Communication::ConnectionVector connections_;
		Foundation::Framework* framework_;
	};

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_Provider_h
