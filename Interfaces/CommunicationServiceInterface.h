#ifndef incl_Interfaces_ComminicationServiceInterface_h
#define incl_Interfaces_ComminicationServiceInterface_h

#include "CoreStdIncludes.h"
#include "ServiceInterface.h"

// TODO: Add ContactListInterface class 
// TODO: Add ConnectionStateEventInterface 
// TODO: Add ContactInfoInterface
// TODO: Add CredentialsInterface

/*
 * ----------------------------------------------------------------------------------
 * CommunicationServiceInterface : Interfaces for to use CommunicationModule services
 * ----------------------------------------------------------------------------------
 */

namespace Communication
{

	// A single contact information of individual contact.
	// todo: should be renamed to ContactAddress ?
	// todo: separate interface and implementation
	// keys: 
	// * protocol
	// * account
	class ContactInfo
	{
	public: 
		virtual ~ContactInfo() {};
		virtual void SetProperty(std::string key, std::string value);
		virtual std::string GetProperty(std::string key);
		virtual std::vector<std::string> GetProperties();
	protected:
		std::map<std::string, std::string> properties_;
	};
	typedef boost::shared_ptr<ContactInfo> ContactInfoPtr;
	typedef std::vector<ContactInfoPtr> ContactInfoList;
	typedef boost::shared_ptr<ContactInfoList> ContactInfoListPtr;

	// presence status of one individual contact
	// todo: online_status {unknow, online, offline}
	//       online_message string
	//       something extra?
	//     * could these be just properties ?
	class PresenceStatusInterface
	{
	public:
		virtual ~PresenceStatusInterface() {};
		virtual void SetOnlineStatus(std::string status) = 0;
		virtual std::string GetOnlineStatus() = 0;
		virtual void SetOnlineMessage(std::string message) = 0;
		virtual std::string GetOnlineMessage() = 0;
		virtual std::vector<std::string> GetOnlineStatusOptions() = 0;
	};
	typedef boost::shared_ptr<PresenceStatusInterface> PresenceStatusPtr;

	// Individual contact in contact list 
	// todo: presence status should only exist with friend contact: -> IsFriend() ?
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

	// A participiant of session
	// todo: add information about communication methods available ?
	class ParticipantInterface
	{
	public:
		virtual ~ParticipantInterface() {};
		virtual ContactPtr GetContact() = 0;
//		virtual IMMessageListPtr GetIMMessages() = 0;
//		virtual std::string GetLastMessageTimeStamp() = 0;
//		virtual int GetMessageCount() = 0;
	};
    typedef boost::shared_ptr<ParticipantInterface> ParticipantPtr;
	typedef std::vector<ParticipantPtr> ParticipantList;
	typedef boost::shared_ptr<ParticipantList> ParticipantListPtr;

	/*
	 * Friend request from IM server
	 */
	class FriendRequestInterface
	{
	public:
		virtual ~FriendRequestInterface() {};
		virtual ContactInfoPtr GetContactInfo() = 0;
		virtual void Accept() = 0;
		virtual void Deny() = 0;
		// virtual std::string GetTimeStamp() = 0;
	};

	typedef boost::shared_ptr<FriendRequestInterface> FriendRequestPtr;
	typedef std::vector<FriendRequestPtr> FriendRequestList;
	typedef boost::shared_ptr<FriendRequestList> FriendRequestListPtr;

	// Communication session (IM, voip, video, etc.)
	// 
	// Send Events:
	// * InvitationsResponse(bool)
	// * ParticipientLeft(Participient)
	// * ParticipientJoined(Participient)
	class SessionInterface
	{
	public:
		virtual ~SessionInterface() {};
		virtual void SendInvitation(ContactPtr c) = 0;
		virtual void Kick(ParticipantPtr p) = 0;
		virtual void Close() = 0;
		virtual ParticipantListPtr GetParticipants() = 0;
		virtual std::string GetProtocol() = 0;
	};
	typedef boost::shared_ptr<SessionInterface> SessionPtr;

	class MessageInterface
	{
	public:
		virtual ~MessageInterface() {};
		virtual std::string GetTimeStamp() = 0; // todo: change to proper timestamp type
		virtual ParticipantPtr GetAuthor() = 0; // todo: or would ContactPtr be better ?
//		virtual std::string GetSessionId() = 0; // todo: Do we need this ??
//		virtual bool IsPrivate() = 0;
	private:
	};

	// chat message
	// todo: more components like attachment (link, file, request, etc.)
	class IMMessageInterface //: public SessionInterface // Would be nice to inherit from MessageInterfa but that causes problems on implementation side
	{
	public:
		virtual ~IMMessageInterface() {};
		virtual void SetText(std::string text) = 0;
		virtual std::string GetText() = 0;

		// These are from MessageInterface class
		virtual std::string GetTimeStamp() = 0; // todo: change to proper timestamp type
		virtual ParticipantPtr GetAuthor() = 0; // todo: or would ContactPtr be better ?
	};
	typedef boost::shared_ptr<IMMessageInterface> IMMessagePtr;
	typedef std::vector<IMMessagePtr> IMMessageList;
	typedef boost::shared_ptr<IMMessageList> IMMessageListPtr;

	// Text chat session
	// Send Events:
	// * MessageReceived
	class IMSessionInterface
	{
	public:
		virtual ~IMSessionInterface() {};
		virtual void SendIMMessage(IMMessagePtr m) = 0;
		virtual IMMessageListPtr GetMessageHistory() = 0;

		// from SessionInterface
		virtual void SendInvitation(ContactPtr c) = 0;
		virtual void Kick(ParticipantPtr p) = 0;
		virtual void Close() = 0;
		virtual ParticipantListPtr GetParticipants() = 0;
		virtual std::string GetProtocol() = 0;
	};
	typedef boost::shared_ptr<IMSessionInterface> IMSessionPtr;
	typedef std::vector<IMSessionPtr> IMSessionList;
	typedef boost::shared_ptr<IMSessionList> IMSessionListPtr;
	
	// Login information for communication server connection
	// todo: rename to Account ?
	// todo: separate interface and implementation
	//
	// info: properties are communication manager and protocol specific:
	// eg.
	// * protocol
	// * account
	// * server
	// * server_port
	// * password
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

	// todo: possibility to join chat rooms
	// Send Events:
	// * SessionRequest (todo: rename to Session?)
	// * PresenceUpdate
	// * ConnectionStateUpdate
	// * MessageReceived (maybe we will not allow messages without session? instead we'll always create one)
	//   todo: rename to Message ?
	// * FriendRequest
	class CommunicationServiceInterface : public Foundation::ServiceInterface
	{
	public:
		virtual ~CommunicationServiceInterface() {}
		virtual void OpenConnection(CredentialsPtr c) = 0;
		virtual void CloseConnection() = 0; 
		virtual IMSessionPtr CreateIMSession(ContactPtr contact) = 0;
		virtual IMSessionPtr CreateIMSession(ContactInfoPtr contact) = 0;
		virtual ContactListPtr GetContactList() = 0;
		virtual void SetPresenceStatus(PresenceStatusPtr p) = 0; 
		virtual PresenceStatusPtr GetPresenceStatus() = 0;
		virtual IMMessagePtr CreateIMMessage(std::string text) = 0;
		virtual void SendFriendRequest(ContactInfoPtr contact_info) = 0;  // todo: move to ContactList class
		virtual void RemoveContact(ContactPtr contact) = 0; // todo: move to ContactList class
	};
	typedef boost::shared_ptr<CommunicationServiceInterface> CommunicationServicePtr;


	/*
	 * All events of COMMUNICATION event group
	 */
    namespace Events
    {
        static const Core::event_id_t PRESENCE_STATUS_UPDATE = 1;
        static const Core::event_id_t IM_MESSAGE = 2;
        static const Core::event_id_t IM_SESSION_REQUEST = 3;
		static const Core::event_id_t FRIEND_REQUEST = 5;
		static const Core::event_id_t FRIEND_RESPONSE = 6;
		static const Core::event_id_t CONNECTION_STATE = 7; 
		static const Core::event_id_t SESSION_STATE = 8; 

		// future events ?
//		static const Core::event_id_t SESSION_INVITATION_RESPONSE_RECEIVED = 5;
//      static const Core::event_id_t FRIENDSHIP_RESPONSE_RECEIVED = 9;
//      static const Core::event_id_t SESSION_JOIN_REQUEST_RECEIVED = 11;

		/*
		 *
		 */
		class PresenceStatusUpdateEventInterface  // : public Foundation::EventDataInterface
		{
		public:
			virtual ~PresenceStatusUpdateEventInterface() {};
			virtual ContactPtr GetContact() = 0;
		};
		typedef boost::shared_ptr<PresenceStatusUpdateEventInterface> PresenceStatusUpdateEventPtr;

		/*
		 *
		 */
		class IMMessageEventInterface //  : public Foundation::EventDataInterface  // PROBLEM: EventDataInterface is not declared yet ?
		{
		public:
			virtual IMMessagePtr GetIMMessage() = 0;
			virtual IMSessionPtr GetSession() = 0;
		};
		typedef boost::shared_ptr<IMMessageEventInterface> IMMessageEventPtr;

		/*
		 *
		 */
		class IMSessionRequestEventInterface // : public Foundation::EventDataInterface
		{
		public:
			virtual ContactPtr GetContact() = 0;
			virtual IMSessionPtr GetSession() = 0;
		};
		typedef boost::shared_ptr<IMSessionRequestEventInterface> IMSessionRequestEventPtr;

		/*
		 *
		 */
		class FriendRequestEventInterface  //: public Foundation::EventDataInterface
		{
		public:
			virtual FriendRequestPtr GetFriendRequest() = 0;
		};
		typedef boost::shared_ptr<FriendRequestEventInterface> FriendRequestEventPtr;

		/*
		 *
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

		/*
		 *
		 */
		class ConnectionStateEventInterface
		{
		public:
			static const int CONNECTION_OPEN = 1;
			static const int CONNECTION_CLOSE = 2;
			static const int CONNECTION_STATE_UPDATE = 3;

			virtual int GetType() = 0;
		};
		typedef boost::shared_ptr<ConnectionStateEventInterface> ConnectionStateEventPtr;

	}

} // end of namespace: Communication

#endif // incl_Interfaces_ComminicationServiceInterface_h
