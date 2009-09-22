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

//#include "TpCommunicationManager.h"

#include "Communication.h"
#include "Credentials.h"
#include "User.h"
#include "TextChatSession.h"
#include "FriendRequest.h"
#include "Contact.h"

namespace TpQt4Communication
{

	#define IM_PROTOCOL "jabber"

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

		TextChatSessionPtr CreateTextChatSession(Contact* contact);
		void CreateVoipSession();
		void SendFriendRequest(Address to, std::string message = "");

		FriendRequestList GetFriendRequests();
		TextChatSessionRequestVector GetTextChatSessionRequests();

	private :
		void ConnectTpSignals();

		std::string id_;
		User* user_;
		State state_;
		std::string protocol_;
		std::string server_;
		Tp::ConnectionPtr tp_connection_;
		std::vector<FriendRequest*> received_friend_requests_;
		//std::vector<TextChatSessionRequest*> received_text_chat_requests_;
		TextChatSessionRequestVector received_text_chat_requests_;
		std::string error_message_;

	public Q_SLOTS:
		void OnConnectionCreated(Tp::PendingOperation *op);
		void OnConnectionReady(Tp::PendingOperation *op);
		void OnConnectionConnected(Tp::PendingOperation *);
		void OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);
		void OnContactRetrieved(Tp::PendingOperation *);
		void OnPresencePublicationRequested(const Tp::Contacts &);
		void OnNewChannels(const Tp::ChannelDetailsList&);

		//! Triggered when TextChannel object is created
		//void OnTextChannelCreated(Tp::PendingOperation* op);

	signals:
		void Connecting(QString &message);
		void Connected();
		void Error(QString &reason);
		void Closed();
		void ReceivedTextChatSessionRequest(TextChatSessionRequest* request);
//		void VoipSessionRequest(VoipSessionRequest* request);
	};
	typedef boost::weak_ptr<Connection> ConnectionWeakPtr;
	typedef std::vector<Connection*> ConnectionVector;

} // end of namespace: TpQt4Communication

#endif // incl_Communication_Connection_h