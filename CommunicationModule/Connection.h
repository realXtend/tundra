#ifndef incl_Communication_Connection_h
#define incl_Communication_Connection_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include <QObject>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/PendingConnection>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingContacts>

#include "Communication.h"
#include "Credentials.h"
#include "User.h"
#include "ChatSession.h"
#include "FriendRequest.h"
#include "ChatSessionRequest.h"
#include "Contact.h"

namespace TpQt4Communication
{
	#define IM_PROTOCOL "jabber"

	typedef std::vector<std::string> PresenceStatusOptions;
	/**
	 *  Connection to IM server like jabber.org. 
	 *  Connection objects are created by calling CommunicationManager.OpenConnection(credentials);

	 *  Connection have to close by calling connection.Close(). All connections will be closed if 
	 *  CommunicationManager is deleted.
	 * 
	 *
	 */
	class Connection : QObject
	{
		Q_OBJECT

		friend class CommunicationManager;

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	public:
		Connection(const Credentials &credentials);
		~Connection();
		enum State{STATE_CONNECTING, STATE_OPEN,  STATE_CLOSED, STATE_ERROR};
		User* GetUser();
//		std::string GetID();
		std::string GetServer();
		void Close();
		std::string GetProtocol();

		//! Return connection state
		State GetState();

		//! Returns. Available presence status options
		PresenceStatusOptions GetAvailablePresenceStatusOptions();

		//! Open new text chat session with given contact
		//! /throws QString object as error message if doesn't success
		ChatSessionPtr CreateChatSession(Contact* contact);

		//! Initialize a new voip session
		//! NOT IMPLEMENTED YQT
		void CreateVoipSession();

		//! Send a friend request to given address
		//! optional message is send if supported by used IM protocol
		//! If friend request is accepted by target then contact list will
		//! be automatically updated on User object and related signal will be emited.
		//! If friend request is rejected there is no way to find it out
		void SendFriendRequest(const Address &target, const std::string &message = "");

		//! Return all received fried requests
		FriendRequestVector GetFriendRequests();

		//! Return all received chat session requests
		ChatSessionRequestVector GetChatSessionRequests();

		//! Return all active chat sessions
		ChatSessionVector GetChatSessions();

	private :

		//! Connect signals of tp_connection_ object
		void ConnectTpSignals();

		//! Check the given telepathy contacts and creates Contact and Friend request objects
		void HandleNewContacts();

		//! id of this connection
		std::string id_;

		//! User aka. account who owns this connection
		User* user_;

		//! Connection state
		State state_;

		//! Protocol
		std::string protocol_;

		//! IM server of this connection
		std::string server_;

		//! Telepathy Connection object
		Tp::ConnectionPtr tp_connection_;

		//! All friend reqeusts received from IM server
		//! These requests have been signaled
		std::vector<FriendRequest*> received_friend_requests_;

		//! All text_chat_sessions
		ChatSessionVector text_chat_sessions_;

		//! All received chat sessions requests
		ChatSessionRequestVector received_text_chat_requests_;

		//! /todo: rename to error_readon_;
		std::string error_message_;

		PendingFriendRequestVector pending_friend_requests_;

	public Q_SLOTS:
		void OnConnectionCreated(Tp::PendingOperation *op);
		void OnConnectionReady(Tp::PendingOperation *op);
		void OnConnectionConnected(Tp::PendingOperation *);
		void OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);
		void OnPresencePublicationRequested(const Tp::Contacts &);
		void OnNewChannels(const Tp::ChannelDetailsList&);

		//! Emits signal ReceivedFriendRequest
		void OnIncomingChatSessionRequestReady(ChatSessionRequest* request);
		void OnPendingFriendRequestReady(PendingFriendRequest* request, PendingFriendRequest::Result result);

	signals:
		void Connecting(QString &message);
		void Connected();
		void Error(QString &reason);
		void Closed();
		void ReceivedChatSessionRequest(ChatSessionRequest* request);
		void ReceivedFriendRequest(FriendRequest* request);
		void FriendRequestRejected(const QString &to) const;
	};
	typedef boost::weak_ptr<Connection> ConnectionWeakPtr;
	typedef std::vector<Connection*> ConnectionVector;

} // end of namespace: TpQt4Communication

#endif // incl_Communication_Connection_h
