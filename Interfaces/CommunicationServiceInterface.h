#ifndef incl_Interfaces_ComminicationServiceInterface_h
#define incl_Interfaces_ComminicationServiceInterface_h

#include "CoreStdIncludes.h"
#include "ServiceInterface.h"

/**
 * Communication service
 *
 * This interface offers Instant Message communication services like chat sessions
 * contact list with presence status information.
 *
 * The main functionality is in CommunicationServiceInterface class. You can require the
 * service via frameworks GetService method with parameter ST_Communication
 *
 * For more information see CommunicationServiceInterface description.
 *
 * @todo Add ContactListInterface class 
 * @todo Add ContactInfoInterface
 * @todo Add CredentialsInterface
 *
 */

namespace Communication
{
	/**
	 * Single contact information of individual contact in contact list.
	 * Contact info contains properties (key,value pairs) eg. 
	 * "protocol","jabber"
	 * "address","your_jabber_address"
	 *
	 * @todo should be renamed to ContactAddress ?
	 * @todo separate interface and implementation
	 */
	class ContactInfo
	{
	public: 
		virtual ~ContactInfo() {};

		//! Set contact info property
		//! @param key name of property eg. "protocol"
		//! @param value value of property eg. "jabber"
		virtual void SetProperty(std::string key, std::string value);

		//! Get contact info property
		//! @param key name of paroperty eg. "protocol"
		virtual std::string GetProperty(std::string key);

		//! List all property keys
		//! @return all property keys
		virtual std::vector<std::string> GetProperties();

	protected:
		std::map<std::string, std::string> properties_;
	};
	typedef boost::shared_ptr<ContactInfo> ContactInfoPtr;
	typedef std::vector<ContactInfoPtr> ContactInfoList;
	typedef boost::shared_ptr<ContactInfoList> ContactInfoListPtr;

	/**
	 *  Presence status of contact in contact list
	 */
	class PresenceStatusInterface
	{
	public:
		virtual ~PresenceStatusInterface() {};

		//! Set the online status eg. {"offline", "away", "online"}
		virtual void SetOnlineStatus(std::string status) = 0;

		//! Get the online status
		virtual std::string GetOnlineStatus() = 0;

		//! Set free status description text
		virtual void SetOnlineMessage(std::string message) = 0;

		//! Get free status description text
		virtual std::string GetOnlineMessage() = 0;

		//! @return options for online status from IM server
		virtual std::vector<std::string> GetOnlineStatusOptions() = 0;
	};
	typedef boost::shared_ptr<PresenceStatusInterface> PresenceStatusPtr;

	/**
	 *  Contact in contact list. Presents an individual person.
	 *  
	 *  @todo Should we support both friend (status subscription allowed) and normal contacts
	 */
	class ContactInterface
	{
	public:
		virtual ~ContactInterface() {};
		virtual void SetName(std::string name) = 0;
		virtual std::string GetName() = 0;
		virtual void AddContactInfo(ContactInfoPtr contact_info) = 0;
		virtual PresenceStatusPtr GetPresenceStatus() = 0;
		virtual ContactInfoListPtr GetContactInfoList() = 0;
		virtual ContactInfoPtr GetContactInfo(std::string protocol) = 0;
	};
	typedef boost::shared_ptr<ContactInterface> ContactPtr;
	typedef std::vector<ContactPtr> ContactList;
	typedef boost::shared_ptr<ContactList> ContactListPtr;

	/**
	 *  Participiant of session. One contact can be paricipant in several sessions
	 *  and participant can basically be someone outside of users contact list 
	 */
	class ParticipantInterface
	{
	public:
		virtual ~ParticipantInterface() {};
		virtual ContactPtr GetContact() = 0;
	};
    typedef boost::shared_ptr<ParticipantInterface> ParticipantPtr;
	typedef std::vector<ParticipantPtr> ParticipantList;
	typedef boost::shared_ptr<ParticipantList> ParticipantListPtr;

	/**
	 *  Friend request from IM server. This can be accepted ot denied. 
	 *  Also the contact info can be asked.
	 */
	class FriendRequestInterface
	{
	public:
		virtual ~FriendRequestInterface() {};

		//! @return contact info for friend request originator
		virtual ContactInfoPtr GetContactInfo() = 0;

		//! Send accept message to IM server
		virtual void Accept() = 0;

		//! Send deny message to IM server
		virtual void Deny() = 0;
	};
	typedef boost::shared_ptr<FriendRequestInterface> FriendRequestPtr;
	typedef std::vector<FriendRequestPtr> FriendRequestList;
	typedef boost::shared_ptr<FriendRequestList> FriendRequestListPtr;

	/**
	 *  Abstract communication session eg. (IM, voip, video, etc.)
	 *
	 *  Send SESSION_STATE events. see SessionStateEventInterface for more information
	 */
	class SessionInterface
	{
	public:
		virtual ~SessionInterface() {};

		//! Not currently supported
		virtual void SendInvitation(ContactPtr c) = 0;

		//! Not currently supported
		virtual void Kick(ParticipantPtr p) = 0;

		//! Closes the session
		virtual void Close() = 0;

		//! @return list of all participants in this sessions
		virtual ParticipantListPtr GetParticipants() = 0;

		//! @return the protocol of this session eg. "jabber"
		//! @todo do we need this?
		virtual std::string GetProtocol() = 0;

		//! @return the originator of this session
		virtual ParticipantPtr GetOriginator() = 0;
	};
	typedef boost::shared_ptr<SessionInterface> SessionPtr;

	/**
	 *  Base class for messages
	 *  @todo Do we need this ?
	 */
	class MessageInterface
	{
	public:
		virtual ~MessageInterface() {};

		//! return local time when message is received in format: "HH::MM"
		virtual std::string GetTimeStamp() = 0; 

		//! @todo would ContactPtr be better because we might have message without sessions? 
		virtual ParticipantPtr GetAuthor() = 0; 
	private:
	};

	/**
	 *  IM message containing text attribute.
	 *
	 * @todo more components like attachment (link, file, request, etc.)
	 * @todo Would be nice to inherit from MessageInterfa but that causes problems on implementation side when 
	 *       we herit same base class twice.
	 */
	class IMMessageInterface //: public SessionInterface 
	{
	public:
		virtual ~IMMessageInterface() {};
		virtual void SetText(std::string text) = 0;
		virtual std::string GetText() = 0;

		//! @fix These are from MessageInterface class. We should remove MessageInterface or inherit from it
		virtual std::string GetTimeStamp() = 0; 
		virtual ParticipantPtr GetAuthor() = 0; 
	};
	typedef boost::shared_ptr<IMMessageInterface> IMMessagePtr;
	typedef std::vector<IMMessagePtr> IMMessageList;
	typedef boost::shared_ptr<IMMessageList> IMMessageListPtr;

	/**
	 *  IM session for receiving and sendint text messages
	 *  You can create IMSessionPtr by asking from CommunicationService.
	 */
	class IMSessionInterface
	{
	public:
		virtual ~IMSessionInterface() {};

		//! Send message to all participants in session
		virtual void SendIMMessage(IMMessagePtr m) = 0;

		//! @return entire list of messages reiceived or sended at this session
		virtual IMMessageListPtr GetMessageHistory() = 0;

		//! from SessionInterface: We should remove SessionInterface class or inherit from it
		virtual void SendInvitation(ContactPtr c) = 0;
		virtual void Kick(ParticipantPtr p) = 0;
		virtual void Close() = 0;
		virtual ParticipantListPtr GetParticipants() = 0;
		virtual std::string GetProtocol() = 0;
		virtual ParticipantPtr GetOriginator() = 0;

	};
	typedef boost::shared_ptr<IMSessionInterface> IMSessionPtr;
	typedef std::vector<IMSessionPtr> IMSessionList;
	typedef boost::shared_ptr<IMSessionList> IMSessionListPtr;
	
	/**
	 *  Interface for credential for login to IM server
	 *
	 *  @todo this is not in use yet, instead we currently use Credentials class
	 */
	class CredentialsInterface
	{
	public: 
		virtual ~CredentialsInterface() {};
		virtual void SetProperty(const std::string &key, const std::string &value) const = 0;
		virtual std::string GetProperty(const std::string &key) const = 0;
		virtual std::vector<std::string> GetProperties() const = 0;
	};

	/**
	 *  Login information for IM server connection login
	 *
	 *  @todo rename to Account or AccountData?
	 *  @todo separate interface and implementation
	 *
	 *  properties are communication manager and protocol specific. You can request
	 *  CredentialsPtr object from CommunicationService. That object has all properties
	 *  set with empty values.
	 *  eg.
	 *  - "protocol"
	 *  - "account"
	 *  - "server"
	 *  - "server_port"
	 *  - "password"
	 */
	class Credentials
	{
	public: 
		virtual ~Credentials() {};
		virtual void SetProperty(std::string key, std::string value);
		virtual std::string GetProperty(std::string key);
		virtual std::vector<std::string> GetProperties();
	protected:
		std::map<std::string, std::string> properties_;
	};
	typedef boost::shared_ptr<Credentials> CredentialsPtr;

	/**
	 * This the base class of communication framework. 
	 *
	 * To use communication functionality you must open connection to IM server.
	 * This is done with OpenConnection method.
	 *
	 * After connection is established you can get contact list and start IM sessions.
	 * You will also be able to receive events for incoming IM sessions and presence 
	 * updated. You are also able to set your own presence information.
	 *
	 * Current implementation supports Jabber protocol but interface is protocl free
	 * and more protocol support is easy to add later.
	 *
	 * @todo Add possibility to join chat rooms
	 * @todo Multiuser IM sessions
	 * @todo voip sessions
	 * @todo video sessions
	 *
	 * Sends COMMUNICATION event group events
	 */
	class CommunicationServiceInterface : public Foundation::ServiceInterface
	{
	public:
		virtual ~CommunicationServiceInterface() {}

		//! Opens the connection to IM server
		virtual void OpenConnection(CredentialsPtr c) = 0;

		//! Closes the connection to IM server
		virtual void CloseConnection() = 0; 

		//! Creates new IM session object
		virtual IMSessionPtr CreateIMSession(ContactPtr contact) = 0;

		//! Creates new IM session object
		//! @todo this is not supported yet. Currently we allow only sessions with contacts in contact list
		virtual IMSessionPtr CreateIMSession(ContactInfoPtr contact) = 0;

		//! @return users contact list from IM server
		virtual ContactListPtr GetContactList() const = 0;

		//! Sets users presence status
		virtual void SetPresenceStatus(PresenceStatusPtr p) = 0; 

		//! Get users own presence status
		//! To change the presence status use methods of returned object
		virtual PresenceStatusPtr GetPresenceStatus() = 0;

		//! Constructs an IM message with given text
		//! The orginator of this message will be the user
		virtual IMMessagePtr CreateIMMessage(std::string text) = 0;

		//! Send friend request. If target accepts the request he sends
		//! an another friend request back and user can accpet/deny that
		virtual void SendFriendRequest(ContactInfoPtr contact_info) = 0;  // todo: move to ContactList class ?

		//! Removen given contact from contact list on IM server
		virtual void RemoveContact(ContactPtr contact) = 0; // todo: move to ContactList class ?

		/**
		 *  Return CredentialsPtr object with necessary properties set with ampty values
		 */
		virtual CredentialsPtr GetCredentials() = 0; 
	};
	typedef boost::shared_ptr<CommunicationServiceInterface> CommunicationServicePtr;


	/**
	 * The events in COMMUNICATION event group.
	 */
    namespace Events
    {
        static const Core::event_id_t PRESENCE_STATUS_UPDATE = 1; //! When presence status of contact in contact list changes
        static const Core::event_id_t IM_MESSAGE = 2;             //! When IMMessage is received
        static const Core::event_id_t IM_SESSION_REQUEST = 3;     //! When IM session request is received
		static const Core::event_id_t FRIEND_REQUEST = 5;         //! When friend request is received
		static const Core::event_id_t FRIEND_RESPONSE = 6;        //! Isn't in use currently
		static const Core::event_id_t CONNECTION_STATE = 7;       //! When session status is changed or contact list content is changed
		static const Core::event_id_t SESSION_STATE = 8;          //! When session ends or participant joins or leaf 


		//! Future events ?
		//! - SESSION_INVITATION_RESPONSE_RECEIVED 
		//! - FRIENDSHIP_RESPONSE_RECEIVED
		//! - SESSION_JOIN_REQUEST_RECEIVED

		/**
		 *  ContactPtr object which presence status have changed
		 */
		class PresenceStatusUpdateEventInterface  // : public Foundation::EventDataInterface
		{
		public:
			virtual ~PresenceStatusUpdateEventInterface() {};
			virtual ContactPtr GetContact() = 0;
		};
		typedef boost::shared_ptr<PresenceStatusUpdateEventInterface> PresenceStatusUpdateEventPtr;

		/**
		 *  Delivers IMMessagePtr object of received message in IMMessage session
		 */
		class IMMessageEventInterface //  : public Foundation::EventDataInterface  // PROBLEM: EventDataInterface is not declared yet ?
		{
		public:
			virtual IMMessagePtr GetIMMessage() = 0;
			virtual IMSessionPtr GetSession() = 0;
		};
		typedef boost::shared_ptr<IMMessageEventInterface> IMMessageEventPtr;

		/**
		 *  Delivers incoming session onject and contact object who is sending the request
		 */
		class IMSessionRequestEventInterface // : public Foundation::EventDataInterface
		{
		public:
			virtual ContactPtr GetContact() = 0;
			virtual IMSessionPtr GetSession() = 0;
		};
		typedef boost::shared_ptr<IMSessionRequestEventInterface> IMSessionRequestEventPtr;

		/**
		 *  Delivers friendrequest object
		 */
		class FriendRequestEventInterface  //: public Foundation::EventDataInterface
		{
		public:
			virtual FriendRequestPtr GetFriendRequest() = 0;
		};
		typedef boost::shared_ptr<FriendRequestEventInterface> FriendRequestEventPtr;

		/**
		 *  Delivers information about session state changes
		 */
		class SessionStateEventInterface
		{
		public:
			static const int SESSION_BEGIN = 1;
			static const int SESSION_END = 2;
			static const int PARTICIPANT_JOINED = 3;
			static const int PARTICIPANT_LEFT = 4;

			virtual int GetType() = 0;
			virtual IMSessionPtr GetIMSession() = 0;
		};
		typedef boost::shared_ptr<SessionStateEventInterface> SessionStateEventPtr;

		/**
		 *  Deliveres information about IM server connection state changes
		 */
		class ConnectionStateEventInterface
		{
		public:
			static const int CONNECTION_OPEN = 1;
			static const int CONNECTION_CLOSE = 2;
			static const int CONNECTION_STATE_UPDATE = 3;
            static const int CONNECTION_CONNECTING = 4;

			virtual int GetType() = 0;
		};
		typedef boost::shared_ptr<ConnectionStateEventInterface> ConnectionStateEventPtr;

	}

} // end of namespace: Communication

#endif // incl_Interfaces_ComminicationServiceInterface_h
