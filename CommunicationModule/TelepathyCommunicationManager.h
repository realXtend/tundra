#pragma once

#include "CommunicationService.h"

// TODO: Need to add dependencies
//#include <telepathy-glib/connection-manager.h>
//#include <telepathy-glib/connection.h>
//#include <telepathy-glib/contact.h>
//#include <telepathy-glib/channel.h>


namespace Communication
{

// Instant messaging sessionj
class Session : IIMSession
{

};

// ICommunicationService implementation using Telepathy library
class TelepathyCommunicationManager : public ICommunicationService
{
public:
	TelepathyCommunicationManager(void);
	~TelepathyCommunicationManager(void);

		void OpenConnection(Credentials c);
		void CloseConnection();
//		void CloseAllConnections(); // if multiple connection allowed
		ISession CreateSession();

private:
//	Conn
};

} // end of namespace Communication
