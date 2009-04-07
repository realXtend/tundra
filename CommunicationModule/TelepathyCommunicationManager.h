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
class Session : ISession, IIMSession
{

};

// ICommunicationService implementation using Telepathy library
class TelepathyCommunicationManager :
	public ICommunicationService
{
public:
	TelepathyCommunicationManager(void);
	~TelepathyCommunicationManager(void);
};

} // end of namespace Communication
