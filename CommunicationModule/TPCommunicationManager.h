#ifndef incl_TpCommunicationManager_h
#define incl_TpCommunicationManager_h
#include "StableHeaders.h"
#include "Foundation.h"
#include "ModuleInterface.h" // for logger

#include <QObject>
#include <QtGui>
#include <QSharedPointer>

#include <TelepathyQt4/Debug__>
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/PendingConnection>
#include <TelepathyQt4/PendingReady>

/**
 * Objects:
 * 
 * - CommunicationManager
 *   - connections
 * - Connection
 *   - sessions
 *   - user
 * - Session
 *   - participants
 * - ContactList(needed?)
 *   - contacts
 * - Contact
 *   - address
 *   - presence_status
 *   - real_name
 * - User
 *   - contact_list
 *   - presence_status
 * - Participant
 *   - address
 *   - message_history (needed?)
 *   
 *
 *
 * Events: All events are sent using framework's event system
 * - TextMessage(address_from, message)
 * - ChatMessage(session_id, address_from, message)
 * 
 * - ConnectionClosed(connection_id)
 *
 * - SessionClosed(session_id)
 * - ParticipantJoined(session_id, participant_id)
 * - ParticipantLeft(session_id, participant_id)
 *
 * - FriendRequest(address_from)
 * - FriendRequestAccepted(address_from)
 * - FriendRequestRejected(address_from)
 *
 * - PresenceUpdate(contact_id, status_text, message_text);
 */ 
namespace TpQt4Communication
{
	typedef std::string Address;
	typedef std::string Protocol;
	typedef std::string PresenceState;
	typedef std::string PresenceMessage;

	/**
	 *  Presence status of contact in contact list or current user 
	 *
	 */
	class PresenceStatus
	{
	public:
		PresenceStatus();

		void SetStatusText(std::string text);
		std::string GetStatusText();
		void SetMessageText(std::string text);
		std::string GetMessageText();
	private:
		std::string status_text_;
		std::string message_text_;
	};
	typedef boost::weak_ptr<PresenceStatus> PresenceStatusWeakPtr;

	///**
 //    * IM address holder for exp. jabber addres: "yourname@jabber.org".
	// * Includes also type of address like: "jabber", "email"
	// * 		
	// */
	//class Address
	//{
	//public:
	//	Address(std::string type, std::string address);
	//	std::string ToString();
	//	std::string GetType();
	//private:
	//	std::string address_;
	//	std::string type_;
	//};
	//typedef boost::weak_ptr<Address> AddressWeakPtr;

	/**
	 *  Contact item in user's contact lisdt
	 *
	 */
	class Contact
	{
	public:
		Contact(Address address, std::string real_name);
		Address GetAddress();
		std::string GetRealName();
		PresenceStatusWeakPtr GetPresenceStatus();
		void SetRealName(std::string name);
	private:
		Address address_;
		std::string real_name_;
		PresenceStatusWeakPtr presence_status_;
	};

	/**
	 * User's contact list aka Friendlist.
	 * Current implementation is flat but might be better to change to support hierarcy.
	 * EVENTS: 
	 *	- FriendRequestResponse
     * - FriendRequest
	 */
	class ContactList
	{
	public:
		
	};


	/**
	 *  An participant is session. A Participant object is created when invited person have
	 *  accepted to join to session.
	 *
	 * You can get an Participant object by calling 
	 *
	 */
	class Participant
	{
	public:
		std::string GetRealName();
		std::string GetAddress();
	};

	/**
	EVENTS:
		- ParticipantLeft
		- ParticipantJoined
		- TextMessageReceived
		- SessionClosed
	*/
	class TextChatSession
	{
	public:
		void Invite(Address a);
		void SendTextMessage(std::string text);
		void Close();
		void GetMessageHistory(); // todo: return value

	};
	typedef boost::shared_ptr<TextChatSession> TextChatSessionPtr;
	typedef boost::weak_ptr<TextChatSession> TextChatSessionWeakPtr;

	/**
	EVENTS: 
		- ParticipantLeft
		- ParticipantJoined
		- TextMessageReceived
		- SessionClosed
	*/
	class VoipSession
	{
	public:
		void Invite(Address a);
		void Close();
		void GetParticipants();
		void SetMicrophoneVolume(double value);
		double GetMicrophoneVolume();
	};

	class VideoConferenceSession
	{

	};

	/**
	 *  Represents an user in one IM server. 
	 *
	 *  User object is created when a connection to IM server is made. You can get 
	 *  User object by calling connection.GetUser()
	 *
	 */
	class User
	{
	public:
		User();
		std::string GetUserID();
		std::string GetProtocol();
		PresenceStatus* GetPresenceStatus();
	private:
		std::string user_id_;
		std::string protocol_;
		PresenceStatus presence_status_;
	};
	typedef boost::weak_ptr<User> UserWeakPtr;

	/**
	 * 
	 *
	 *
	 */
	class FriendRequest
	{
	public:
		enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED };
		Address GetAddressFrom();
		Address GetAddressTo();
		State GetState();
	private:
		State state_;
		Address from_;
		Address to_;
	};

	class TextChatRequest
	{
	public:
		enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED };
		State GetState();
	private:
		State state_;
	};

	/**
	 *  Credential for connecting to IM server
	 *
	 *  NOTE: Propably attribute map with arbitrary keys value pairs are the best choise here...
	 *
	 */
	class Credentials
	{
	public:
		Credentials();
		void SetProtocol(std::string protocol);
		void SetUserID(std::string user_id);
		void SetPassword(std::string pwd);
		void SetServer(std::string server);
		std::string GetProtocol();
		std::string GetUserID();
		std::string GetUserPassword();
		std::string GetServer();
		void LoadFromFile(std::string path);
	private:
		std::string user_id_;
		std::string password_;
		std::string protocol_;
		std::string server_;
		std::map<std::string, std::string> attributes_;
	};
	typedef boost::weak_ptr<Credentials> CredentialsWeakPtr;

	/**
	 *  Connection to IM server like jabber.org. 
	 *  Connection objects are created by calling CommunicationManager.OpenConnection(credentials);

	 *  Connection have to close by calling connection.Close(). All connections will be closed if 
	 *  CommunicationManager is deleted.
	 * 
	 *
	 */
//	class CommunicationManager;

	class Connection : QObject
	{
		#define IM_PROTOCOL "jabber"

	//	friend class CommunicationManager;
		Q_OBJECT

	public:
		Connection();
		enum State{STATE_CONNECTING, STATE_OPEN,  STATE_CLOSED, STATE_ERROR};
		void SendFriendRequest(const Address &a);
		User* GetUser();
		std::string GetID();
		void Close();
		std::string GetProtocol();
		State GetState();

		TextChatSessionPtr CreateTextChatSession();
		void CreateVoipSession();
		

	private :
		std::string id_;
		User* user_;
		State state_;
		std::string protocol_;
		Tp::ConnectionPtr tp_connection_;
		std::vector<FriendRequest*> received_friend_requests_;
		std::vector<TextChatRequest*> received_text_chat_requests_;
		std::string error_message_;
		

	public Q_SLOTS:
		void OnConnectionCreated(Tp::PendingOperation *op);
		void OnConnectionConnected(Tp::PendingOperation *op);
		void OnConnectionInvalidated(Tp::PendingOperation *op);

	};
	typedef boost::weak_ptr<Connection> ConnectionWeakPtr;
	typedef std::vector<Connection*> ConnectionList;

	/**
	 * 
	 *
	 *
	 */
	class CommunicationManager : QObject
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS

		#define CONNECTION_MANAGER_NAME "gabble"
	private:
		
	public:
		static const std::string NameStatic() { return "Communicationmanager"; } // for logging functionality
		enum State {STATE_INITIALIZING, STATE_READY, STATE_ERROR};
		CommunicationManager();
		~CommunicationManager();
		static CommunicationManager* GetInstance();
		

		Connection* OpenConnection(const Credentials &credentials);
		State GetState();

		ConnectionList GetAllConnections();
		ConnectionList GetConnections(std::string protocol);
		void CloseAllConnections();

	private:
		bool IsDBusServiceAvailable(std::string name);
		void StartDBusDaemon();
		
		Tp::ConnectionManagerPtr connection_manager_;
		bool connection_manager_ready_;
		//std::list<Tp::ConnectionPtr> connections_;
		ConnectionList connections_;
		Tp::PendingReady* pending_ready_;
		State state_;

		//! dbus daemon and gabble process for Windows platform
		QProcess* dbus_daemon_;

	private Q_SLOTS:
		void OnConnectionManagerReady(Tp::PendingOperation *op);
		void OnDBusDaemonStdout();
		void OnDBusDaemonExited( int exitCode, QProcess::ExitStatus exitStatus );

	public:
		static CommunicationManager* instance_;
	};


} // namespace TpQt4Communication


#endif incl_TpCommunicationManager_h
