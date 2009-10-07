#ifndef incl_Comm_interface_h
#define incl_Comm_interface_h

#include "Foundation.h"
#include <QObject>
#include <QTime>

/**
 * Naali viewer's communication framework
 *
 * This interface provides the basic building blocks for usage of IM services like
 * jabber, irc, opensim chat etc. 
 *
 * The singleton CommunicationService object provides connections to IM servers with 
 * different protocols. Connection object is used to open chat session, obtain friend list
 * and setting presense status. 
 *
 *
 */

namespace CommunicationService
{

	/**
	 *  Gredentials for opening connection to IM server. Some fields are optional in 
	 *  with some protocols
	 *  usage: credentials = new Credentials("jabber", "my_jabber_id", "my_jabber_server", "5222"); 
	 *
	 *  \todo <string><string map might be better here so that we could offer protocol specific fields.
	 *
	 */
	class CredentialsInterface
	{
	public:
		CredentialsInterface(const QString& protocol, const QString& user_id, const QString &server, const QString &port );

		//! eg. "Jabber", "OS_UDP_IM", "irc"
		//!
		void SetProtocol(const QString &protocol);
		QString GetProtocol() const;

		void SetUserID(const QString &user_id);
		QString GetUserID() const;

		void SetServer(const QString &server);
		QString GetServer() const;

		void SetPort(const QString &port);
		QString GetPort() const;
	};

	/**
	 * A contact item on ContactList object
	 *
	 *
	 */
	class ContactInterface : QObject
	{
		Q_OBJECT
	public:
		virtual QString GetID();
		virtual	QString GetName();
		virtual void SetName(const QString& name);
		virtual QString GetPresenceStatus();
		virtual QString GetPresenceMessage();

	signals:
		void PresenceStatusChanged(const QString &status, const QString &message);
	};
	typedef std::vector<ContactInterface*> ContactVector;

	/**
	 * Contact list
	 * /todo: contact group
	 */
	class ContactGroupInterface;
	typedef std::vector<ContactGroupInterface*> ContactGroupVector;
	class ContactGroupInterface
	{
	public:
		//! Provides name of this contact group
		virtual QString GetName() const;
		virtual ContactVector GetContacts() const;
		virtual ContactGroupVector GetSubGroups() const;
	};

	/**
	 * A participant of one ChatSession object. A participant can be a contact or
	 * just a nick on chat room
	 */
	class ChatSessionParticipant
	{
	public:

		//! Return NULL pointer if the participant is not on
		//! contact list
		virtual ContactInterface* GetContact() const;

		virtual QString GetID() const;
		virtual QString GetName() const;
	};

	/**
	 * 
	 *
	 *
	 * \todo GetAttachment
	 */
	class IMMessageInterface
	{
	public:
		virtual ChatSessionParticipant* GetOriginator() const;
		virtual QTime GetTimeStamp() const;
		virtual QString GetText() const;
	};

	/**
	 * Text message based communication session with one or more participants.
	 *
	 *
	 */
	class ChatSessionInterface: QObject
	{
		Q_OBJECT
	public:
		virtual void SendMessage(const QString &text);
	signals:
		void MessageReceived(const QString &text, const ChatSessionParticipant& participant);
	};
	typedef boost::shared_ptr<ChatSessionInterface> ChatSessionPtr;

	/**
	 *
	 *
	 */
	class FriendRequestInterface
	{
	public:
		enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED };
		State GetState();
		void Accept();
		void Reject();
	};
	typedef std::vector<FriendRequestInterface*> FriendRequestVector;

	/**
	 * A connection to IM server
	 *
	 *
	 */
	class ConnectionInterface : QObject
	{
		Q_OBJECT
	public:
		enum State {STATE_INITIALING, STATE_READY, STATE_CLOSED, STATE_ERROR};

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
		virtual ContactGroupInterface GetContacts() const;

		//! Provides a list of availble presence status options to set
		virtual QStringList GetAvailablePresenceStatusOptions() const;

		//! Open new chat session with given contact
		virtual ChatSessionPtr OpenChatSession(const ContactInterface &contact);

		//! Send a friend request to target address
		virtual void SendFriendRequest(const QString &target, const QString &message);

		//! Provides all received friend requests with in this connection session
		//! FriendRequest object state must be checked to find out new ones.
		virtual FriendRequestVector GetFriendRequests() const;

		//! Closes the connection
		virtual void Close();

	signals:
		void ConnectionReady();
		void ConnectionClosed();
		// \todo Contact status updated 
		// \todo Friendship terminated
		void FriendRequestReceived(const FriendRequestInterface& request); 
		void FriendRequestAccepted(const QString &target); 

		//! If the protocol doesn't support this then no
		//! notification is send back about rejecting the friend request
		void FriendRequestRejected(const QString &target); 
	};
	typedef boost::shared_ptr<ConnectionInterface> ConnectionPtr;
	typedef std::vector<ConnectionPtr> ConnectionVector;



	/**
	 * DO WE NEED THIS CLASS TO BE PUBLIC ?
	 *
	 * Provides connections to IM servers eg. to jabber server.
	 * Can support multiple protocols
	 *
	 */
	//class ConnectionProviderInterface
	//{
	//public:
	//	virtual QStringList GetSupportedProtocols() const;
	//	virtual void OpenConnection(const CredentialsInterface& credentials);
	//	virtual ConnectionVector GetConnections() const;
	//};

	/**
	 * Singleton class which provides connections to IM servers using registered connection providers
	 * To get IM connection: 1) setup a credential object 
	 *                       2) call OpenConnection
	 *
	 */
	class CommunicationServiceInterface :QObject
	{
		Q_OBJECT
	public:
	//	virtual void RegisterConnectionProvider(const ConnectionProviderInterface* &provider);
		virtual QStringList GetSupportedProtocols() const;
		virtual ConnectionPtr OpenConnection(const CredentialsInterface &credentials);
		virtual ConnectionVector GetConnections() const;
	signals:
		void NewProtocolSupported(QString protocol);
		void ConnectionOpened(ConnectionPtr connection);
		void ConnectionClosed(ConnectionPtr connection);
	};

} // end of namepace: CommunicationService

#endif incl_Comm_interface_h
