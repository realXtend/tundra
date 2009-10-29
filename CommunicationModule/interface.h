#ifndef incl_Comm_interface_h
#define incl_Comm_interface_h

#include "Foundation.h"
#include <QObject>
#include <QTime>
#include <exception>

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
 * \todo throw exception eg. when some functionality is not supported by some protocol.
 */

namespace Communication
{

	/**
	 *  Gredentials for opening connection to IM server. Some fields are optional in 
	 *  with some protocols
	 *  usage: credentials = new Credentials("jabber", "my_jabber_id", "my_jabber_server", "5222"); 
	 *
	 *  \todo <string><string map might be better here so that we could offer protocol specific fields.
	 *  \todo Also per protocol Credertials classes might be the right way
	 *        eg. JabberCredentials, IrcCredentials, OpensimIMCredentials 
	 */
	class CredentialsInterface
	{
	public:
//		virtual CredentialsInterface(const QString& protocol, const QString& user_id, const QString &server, const QString &port ) = 0;

		//! Define the used protocol eg. "Jabber", "Opensim_UDP_IM", "irc"
		virtual void SetProtocol(const QString &protocol) = 0;

		//! Provides the protocol
		virtual QString GetProtocol() const = 0;

		virtual void SetUserID(const QString &user_id) = 0;
		virtual QString GetUserID() const = 0;

		virtual void SetPassword(const QString &password) = 0;
		virtual QString GetPassword() const = 0;

		virtual void SetServer(const QString &server) = 0;
		virtual QString GetServer() const = 0;

		//! Define port of IM server to connect
		virtual void SetPort(int port) = 0;

		//! Provides the port of IM server
		virtual int GetPort() const = 0;
	};

	/**
	 * A contact item on ContactList object. Presents a real life person.
	 *
	 */
	class ContactInterface : public QObject
	{
		Q_OBJECT
	public:
		//! Provides ID of the contact. This can be eg. "myjabberid@myjabberserver"
		virtual QString GetID() const = 0;

		//! Provides name of this contact item
		virtual	QString GetName() const = 0;

		//! Define name of this contact item
		//! If the protocol doesn't support redefinition of the name then 
		//! this method has affect only on current IM server connection and 
		//! the name is reseted when new connection is established.
		virtual void SetName(const QString& name) = 0; 

		//! Provides presence status of this contact item. 
		//! eg. "chat", "offline", "free"
		virtual QString GetPresenceStatus() const = 0;

		//! Provides presence message of this contact item.
		//! This is freely textual information about current state eg. "At lunch, be back in 30 minutes..." 
		virtual QString GetPresenceMessage() const = 0;

	signals:
		void PresenceStatusChanged(const QString &status, const QString &message);

		//! When contact item has canceled friendship eg. when removed user from (s)hes contact list
		void FriendshipEnded();
	};
	typedef std::vector<ContactInterface*> ContactVector;

	/**
	 *  List of Contact objects eg. a friend list.
	 *  Contact groups can be include Contact and ContactGroup objects.
	 *  ContactGroup object is created by Connection object with information from IM server.
	 *
	 *  Adding or removing items is not allowed. But you can Send a friend request from Connection object
	 *  and remove contact from Connection object.
	 *  
	 */
	class ContactGroupInterface;
	typedef std::vector<ContactGroupInterface*> ContactGroupVector;
	class ContactGroupInterface
	{
	public:
		//! Provides name of this contact group
		virtual QString GetName() const = 0;

		//! Set name for this contact group
		//! If the protocol supports then the given name is saved on server side.
		virtual void SetName(const QString &name) = 0;

		//! Provides all Contact objects on this contact group
		virtual ContactVector GetContacts() = 0;

		//! Priovides all sub groups of this contact groups
		virtual ContactGroupVector GetGroups() = 0;
	signals:
	};

	/**
	 *  A participant of one ChatSession object. A participant can be a contact or
	 *  just a nick on chat room.
	 *
	 *  @todo Add methods to get related scene object if that exist. So that 
	 *        3D in-world chat UI can be made.
	 */
	class ChatSessionParticipantInterface : public QObject
	{
		Q_OBJECT
	public:

		//! @return contact object if the participant have one
		//          otherwise return a NULL pointer 
		virtual ContactInterface* GetContact() const = 0;

		//! @return id of this participant 
		virtual QString GetID() const = 0;

		//! Provides name of this participant
		virtual QString GetName() const = 0;
	signals:
	};
	typedef std::vector<ChatSessionParticipantInterface*> ChatSessionParticipantVector;

	/**
	 *  A message in chat session.
	 *  @note This interface is not currently used!
	 *  @todo GetAttachment() method for supporting file attachement feature in future
	 */
	class ChatMessageInterface
	{
	public:
		virtual ChatSessionParticipantInterface* GetOriginator() const = 0;
		virtual QDateTime GetTimeStamp() const = 0;
		virtual QString GetText() const = 0;
	};
	typedef std::vector<ChatMessageInterface*> ChatMessageVector;

	/**
	 * Text message based communication session with one or more participants.
	 * This can represents irc channel or jabber conversation.
	 *
	 */
	class ChatSessionInterface: public QObject
	{
		Q_OBJECT
	public:
		enum State { STATE_INITIALIZING, STATE_OPEN, STATE_CLOSED, STATE_ERROR };

		virtual ~ChatSessionInterface() {};

		//! Send a text message to chat session
		//! @param text The message
		virtual void SendMessage(const QString &text) = 0;

		//! @return State of the session
		virtual State GetState() const = 0;

		//! Closes the chat session. No more messages can be send or received. 
		//! Causes Closed signals to be emitted.
		virtual void Close() = 0;

		//! @return all known participants of the chat session inlcuding the user
		virtual ChatSessionParticipantVector GetParticipants() const = 0;

		//! @return the message history of this chat sessions
		virtual ChatMessageVector GetMessageHistory() = 0;

	signals:
		//! @todo REMOVE THIS METHOD
		void MessageReceived(const QString &text, const Communication::ChatSessionParticipantInterface& participant);
		void MessageReceived(const ChatMessageInterface &message);
		void ParticipantJoined(const ChatSessionParticipantInterface& participant);
		void ParticipantLeft(const ChatSessionParticipantInterface& participant);
		void Opened(ChatSessionInterface*);
		void Closed(ChatSessionInterface*);
	};

	/**
	 * @todo Design issue: Do we need two separaed participant classes?
	 *
	 */
	class VoiceSessionParticipantInterface : public QObject
	{
		Q_OBJECT
	public:
		//! @return contact object if the participant have one
		//          otherwise return a NULL pointer 
		virtual ContactInterface* GetContact() const = 0;

		//! @return id of this participant 
		virtual QString GetID() const = 0;

		//! Provides name of this participant
		virtual QString GetName() const = 0;
	signals:

	};
	typedef std::vector<VoiceSessionParticipantInterface*> VoiceSessionParticipantVector;

	/**
	 *  \NOTE This is interface is under construction
	 *        More information about audio module planning is needed.
	 *       
	 */
	class VoiceSessionInterface: public QObject
	{
		Q_OBJECT
	public:
		enum State { STATE_INITIALIZING, STATE_OPEN, STATE_CLOSED, STATE_ERROR };

		//! @return State of the session
		virtual State GetState() const = 0;

		//! @return all known participants of the chat session inlcuding the user
		virtual VoiceSessionParticipantVector GetParticipants() const = 0;

		virtual void Close() = 0;
	signals:
		void ParticipantJoined(const VoiceSessionParticipantInterface& participant);
		void ParticipantLeft(const VoiceSessionParticipantInterface& participant);
		void Opened(ChatSessionInterface*);
		void Closed(VoiceSessionInterface*);
	};

	/**
	 * A received friend request. This can be accepted or rejected. If Accpet() methos is called1
	 * then Connection object emits NewContact signal.
	 */
	class FriendRequestInterface : public QObject
	{
		Q_OBJECT
	public:
		enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED };
		virtual QString GetOriginatorName() const = 0;
		virtual QString GetOriginatorID() const = 0;
		virtual State GetState() const = 0;
		virtual void Accept() = 0;
		virtual void Reject() = 0;

	signals:
		//! When target have accepted the friend request
		void FriendRequestAccepted(const FriendRequestInterface* request); 

		//! If the protocol doesn't support this then no
		//! notification is send back about rejecting the friend request
		void FriendRequestRejected(const FriendRequestInterface* request); 
	};
	typedef std::vector<FriendRequestInterface*> FriendRequestVector;

	/**
	 *  A connection to IM server. This class do the most of the work. It provides 
	 *  state information about connection, contact list, allow to set presence status and 
	 *  provides methods to open communication sessions such as chat and voice.
	 *
	 *  It also signals about incoming friend request and communication sessions.
	 *
	 *  @todo Add methods to remove a friend from friend list
	 *  @todo Add method to open voice session
	 *  @todo Add method to open video session
	 */
	class ConnectionInterface : public QObject
	{
		Q_OBJECT
	public:
		//! The state options of Connection object
		//! ConnectionReady and ConnectionClosed signals are emited when state
		//! changes to STATE_OPEN or STATE_CLOSED
		enum State {STATE_INITIALIZING, STATE_OPEN, STATE_CLOSED, STATE_ERROR};

		virtual ~ConnectionInterface() {};

		//! Provides name of the connection
		virtual QString GetName() const = 0;

		//! Connection protocol
		virtual QString GetProtocol() const = 0;

		//! Connection state
		virtual State GetState() const = 0;

		//! Provides server address of this IM server connection
		virtual QString GetServer() const = 0;

		//! Provides textual descriptions about error
		//! If state is not STATE_ERROR then return empty
		virtual QString GetReason() const = 0;

		//! Provides contact list associated with this IM server connection
		virtual ContactGroupInterface& GetContacts() = 0;

		//! @return list of presence status opstions contacts might have
		virtual QStringList GetPresenceStatusOptionsForContact() const = 0;

		//! @return list of presence status options user can set
		virtual QStringList GetPresenceStatusOptionsForSelf() const = 0;

		//! Open new chat session with given contact
		//! @param contact Chat partner target
		virtual ChatSessionInterface* OpenPrivateChatSession(const ContactInterface &contact) = 0;

		//! Open new chat session with given user
		//! @param user_id The user id of chat partner target
		virtual ChatSessionInterface* OpenPrivateChatSession(const QString& user_id) = 0;

		//! Open new chat session to given room
		virtual ChatSessionInterface* OpenChatSession(const QString &channel) = 0;

		//! OPen a new voice chat session with given contact
		//! @param contact Voice chat partner 
		virtual VoiceSessionInterface* OpenVoiceSession(const ContactInterface &contact) = 0;

		//! Send a friend request to target address
		virtual void SendFriendRequest(const QString &target, const QString &message) = 0;

		//! Provides all received friend requests with in this connection session
		//! FriendRequest object state must be checked to find out new ones.
		//! If friend request is not answered the server will resend it on next 
		//! connection
		virtual FriendRequestVector GetFriendRequests() const = 0;

		//! Set presence status state of user
		//! @param status Allowed values are returned by GetPresenceStatusOptionsForSelf methos
		virtual void SetPresenceStatus(const QString &status) = 0;

		//! Set presene status message of user
		//! @param message Any text is accepted
		virtual void SetPresenceMessage(const QString &message) = 0;

		//! Closes the connection
		virtual void Close() = 0;

	signals:
		//! When connection become state where communication sessions can 
		//! be opened and contact list is fethed from server.
		void ConnectionReady(Communication::ConnectionInterface& connection);

		//! When connection is closed by user or server
		void ConnectionClosed(Communication::ConnectionInterface& connection);

		//! When connection state become error
		void ConnectionError(Communication::ConnectionInterface& connection);

		//! When a chat session is initialized by IM server
		void ChatSessionReceived(Communication::ChatSessionInterface& session);

		//! When a voice session is initialized by IM server
		void VoiceSessionReceived(Communication::VoiceSessionInterface& session);

		//! When a new contact is added to contact list
		//! Basically this happens when someone accept friend request
		void NewContact(const ContactInterface& contact);

		//! When contact on contact list removes user from his/her contact list
		//! then that contact will be automatically removed from user's contact list
		void ContactRemoved(const ContactInterface& contact);

		//! When contact status changes
		void ContactStatusUpdated(const ContactInterface& contact);

		//! When a friend request is received from IM server
		void FriendRequestReceived(Communication::FriendRequestInterface& request); 

		//! When target have accepted the friend request
		void FriendRequestAccepted(const QString &target); 

		//! If the protocol doesn't support this then no
		//! notification is send back about rejecting the friend request
		void FriendRequestRejected(const QString &target); 
	};
	typedef std::vector<ConnectionInterface*> ConnectionVector;

	/**
	 *  This class is only used by CommunicationService object. Do not use this
	 *  directly!
     *
	 * Provides connections to IM servers eg. to jabber server.
	 * ConnectionProvide object can support multiple protocols same time.
	 * 
	 * This class is only used by ConnectionModule and ConnectionService objects.
	 */
	class ConnectionProviderInterface : public QObject
	{
		Q_OBJECT
	public:
		virtual ~ConnectionProviderInterface() {};

		//! Provides list of supported protocols by this provider
		virtual QStringList GetSupportedProtocols() const = 0;

		//! Open a new connection to IM server woth given credentials
		virtual ConnectionInterface* OpenConnection(const CredentialsInterface& credentials) = 0;

		//! Provides all Connections objects created by this provider
		virtual ConnectionVector GetConnections() const = 0;
	signals:
		void ProtocolListUpdated(const QStringList &protocols);
		void ConnectionOpened(Communication::ConnectionInterface* connection);
		void ConnectionClosed(Communication::ConnectionInterface* connection);
	};
	typedef std::vector<ConnectionProviderInterface*> ConnectionProviderVector;

	/**
	 * Singleton class which provides connections to IM servers using registered connection providers
	 * To get IM connection: 1) setup a credential object 
	 *                       2) call OpenConnection
	 *
	 */
	class CommunicationServiceInterface : public QObject
	{
		Q_OBJECT
	public:
		virtual ~CommunicationServiceInterface() {};

		//! Static method to provide singleton CommunicationServiceInterface object 
		static CommunicationServiceInterface* GetInstance();

		//! Register a ConnectionProvider object to communication service
		//! Without any connection provider communication service cannot provide
		//! any communication protocols.
		virtual void RegisterConnectionProvider( ConnectionProviderInterface* const provider) = 0;

		//! Provides list of all currently supported protocols
		//! by all registered connection prividers
		virtual QStringList GetSupportedProtocols() const = 0;

		//! Create new Connection object accordingly given credentials
		virtual ConnectionInterface* OpenConnection(const CredentialsInterface &credentials) = 0;

		//! Return all Connection objects
		virtual ConnectionVector GetConnections() const = 0;

		//! Provides Connection objects which supports given protocol
		virtual ConnectionVector GetConnections(const QString &protocol) const = 0;
	signals:
		//! When a new protocol is supported
		void NewProtocolSupported(QString protocol);

		void ProtocolListUpdated(const QStringList &protocols);

		//! When a new connection is opened
		void ConnectionOpened(ConnectionInterface* connection);

		//! When connection is closed
		void ConnectionClosed(ConnectionInterface* connection);
	};

} // end of namepace: Communication

#endif incl_Comm_interface_h
