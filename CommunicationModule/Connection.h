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
		void SendFriendRequest(const Address &a);
		User* GetUser();
//		std::string GetID();
		std::string GetServer();
		void Close();
		std::string GetProtocol();
		State GetState();

		//! Available presence status options
		
		PresenceStatusOptions GetAvailablePresenceStatusOptions();

		//! Open new text chat session with given contact
		//! /throws QString object as error message if doesn't success
		ChatSessionPtr CreateChatSession(Contact* contact);

		//! 
		void CreateVoipSession();
		// void JoinChatRoom(ChatRoomAddress address);

		//! Send a friend request to given address
		//! optionam message is send if supported by used IM protocol
		void SendFriendRequest(Address to, std::string message = "");

		FriendRequestVector GetFriendRequests();
		ChatSessionRequestVector GetChatSessionRequests();
		ChatSessionVector GetChatSessions();

	private :
		void ConnectTpSignals();

		std::string id_;
		User* user_;
		State state_;
		std::string protocol_;
		std::string server_;
		Tp::ConnectionPtr tp_connection_;
		std::vector<FriendRequest*> received_friend_requests_;
		ChatSessionVector text_chat_sessions_;
		ChatSessionRequestVector received_text_chat_requests_;
		std::string error_message_;

	public Q_SLOTS:
		void OnConnectionCreated(Tp::PendingOperation *op);
		void OnConnectionReady(Tp::PendingOperation *op);
		void OnConnectionConnected(Tp::PendingOperation *);
		void OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);
		void OnContactRetrieved(Tp::PendingOperation *);
		void OnPresencePublicationRequested(const Tp::Contacts &);
		void OnNewChannels(const Tp::ChannelDetailsList&);

		//! Emits signal ReceivedFriendRequest
		void OnIncomingChatSessionRequestReady(ChatSessionRequest* request);

	signals:
		void Connecting(QString &message);
		void Connected();
		void Error(QString &reason);
		void Closed();
		void ReceivedChatSessionRequest(ChatSessionRequest* request);
		void ReceivedFriendRequest(FriendRequest* request);
	};
	typedef boost::weak_ptr<Connection> ConnectionWeakPtr;
	typedef std::vector<Connection*> ConnectionVector;

} // end of namespace: TpQt4Communication

#endif // incl_Communication_Connection_h
