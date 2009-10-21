#ifndef incl_Communication_TelepathyIM_Connection_h
#define incl_Communication_TelepathyIM_Connection_h

#include <QStringList>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/PendingConnection>
#include <TelepathyQt4/PendingOperation>
#include "Foundation.h"
#include "..\interface.h"
#include "ContactGroup.h"

namespace TelepathyIM
{
	/**
	 *  Connection to IM server using Telepathy framework
	 *
	 *
	 */
	class Connection : public Communication::ConnectionInterface
	{
		Q_OBJECT
	public:
		Connection(Tp::ConnectionManagerPtr tp_connection_manager, const Communication::CredentialsInterface &credentials);

		virtual ~Connection();

		//! Provides name of the connection
		virtual QString GetName() const;

		//! Connection protocol
		virtual QString GetProtocol() const;

		//! Connection state
		virtual Communication::ConnectionInterface::State GetState() const;

		//! Provides server address of this IM server connection
		virtual QString GetServer() const;

		//! Provides textual descriptions about error
		//! If state is not STATE_ERROR then return empty
		virtual QString GetReason() const;

		//! Provides contact list associated with this IM server connection
		virtual Communication::ContactGroupInterface& GetContacts();

		//! Provides a list of availble presence status options to set
		virtual QStringList GetAvailablePresenceStatusOptions() const;

		//! Open new chat session with given contact
		//! @param contact Chat partner target
		virtual Communication::ChatSessionInterface* OpenPrivateChatSession(const Communication::ContactInterface &contact);

		//! Open new chat session with given user
		//! @param user_id The user id of chat partner target
		virtual Communication::ChatSessionInterface* OpenPrivateChatSession(const QString& user_id);

		//! Open new chat session to given room
		virtual Communication::ChatSessionInterface* OpenChatSession(const QString &channel);

		//! Send a friend request to target address
		virtual void SendFriendRequest(const QString &target, const QString &message);

		//! Provides all received friend requests with in this connection session
		//! FriendRequest object state must be checked to find out new ones.
		//! If friend request is not answered the server will resend it on next 
		//! connection
		virtual Communication::FriendRequestVector GetFriendRequests() const;

		//! Closes the connection
		virtual void Close();
	protected:
		virtual void CreateTpConnection(const Communication::CredentialsInterface &credentials);
		virtual void OnConnectionCreated(Tp::PendingOperation *op);
		virtual void Connection::OnConnectionClosed(Tp::PendingOperation *op);

		Tp::ConnectionManagerPtr &tp_connection_manager_;
		Tp::ConnectionPtr tp_connection_;

		State state_;
		QString name_;
		QString protocol_;
		QString server_;
		QString reason_;
		ContactGroup friend_list_;
	};
	typedef std::vector<Connection*> ConnectionVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_Connection_h
