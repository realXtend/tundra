#ifndef incl_CommunicationServerConnection_h
#define incl_CommunicationServerConnection_h

#include "CoreStdIncludes.h"

#include "CommunicationManagerServiceInterface.h"
#include "CommunicationService.h"

namespace Communication
{

	// events: 
	// - SessionInvitation
	// - PresenceUpdate
	// - ConnectionStateUpdate
	// - Message  {text, request, attachment, ?}
	class CommunicationServerConnection
	{
	public:
		CommunicationServerConnection(void);
		~CommunicationServerConnection(void);

		void Open(Credentials c);
		void Close();
		void OpenSession();
		void SetPresence(PresenceInfo p);
	};



} // end of namespace Communication

#endif // incl_CommunicationServerConnection_h