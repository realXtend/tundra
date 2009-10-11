#ifndef incl_Communication_OSIMConnection_h
#define incl_Communication_OSIMConnection_h

#include "Foundation.h"
#include "..\interface.h"
#include <QStringList>
#include "ChatSession.h"

#define OPENSIM_IM_PROTOCOL "OpensimUDP"

namespace OpensimIM
{

	/**
	 *  A connection presentation to Opensim based world server.
	 *  The actual udp connections is already established when connecting to wrold
	 *  this class just encapsultates the IM functionality of that udp connection
	 *  Close method of this class does not close the underlaying udp connections
	 *  but just set this object to logical closed state
	 */
	class Connection : public Communication::ConnectionInterface
	{
	public:
		Connection(Foundation::Framework* framework);
		virtual ~Connection();

		//! Provides name of the connection
		virtual QString GetName() const;

		//! Connection protocol
		virtual QString GetProtocol() const;

		//! Connection state
		virtual State GetState() const;

		//! Provides server address of this IM server connection
		virtual QString GetServer() const;

		//! Provides textual descriptions about error
		//! If state is not STATE_ERROR then return empty
		virtual QString GetReason() const;

		//! Provides contact list associated with this IM server connection
		virtual Communication::ContactGroupInterface* GetContacts() const;

		//! Provides a list of availble presence status options to set
		virtual QStringList GetAvailablePresenceStatusOptions() const;

		//! Open new chat session with given contact
		virtual Communication::ChatSessionPtr OpenChatSession(const Communication::ContactInterface &contact);

		//! Send a friend request to target address
		virtual void SendFriendRequest(const QString &target, const QString &message);

		//! Provides all received friend requests with in this connection session
		//! FriendRequest object state must be checked to find out new ones.
		//! If friend request is not answered the server will resend it on next 
		//! connection
		virtual Communication::FriendRequestVector GetFriendRequests() const;

		//! Closes the connection
		virtual void Close();

	private:
		Foundation::Framework* framework_;
		QString name_;
		QString protocol_;
		State state_;
		QString server_;
		QString reason_;
		ChatSessionVector chat_sessions_;
	};
	typedef std::vector<Connection*> ConnectionVector;

} // end of namespace: OpensimIM

#endif incl_Communication_OSIMConnection_h
