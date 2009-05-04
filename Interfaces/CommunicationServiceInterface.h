#ifndef incl_Interfaces_ComminicationServiceInterface_h
#define incl_Interfaces_ComminicationServiceInterface_h

#include "CoreStdIncludes.h"
#include "ServiceInterface.h"

/* ---------------------------------------------
 * Interfaces for CommunicationModule usage.
 * ---------------------------------------------
 */

namespace Communication
{

	// A single contact information of individual contact.
	// Eg. jabber id:  "jid": "myjabberid@myjabberprovider.com"
	// todo: should be renamted to ContactAddresd ?
	class ContactInfo
	{
	public:
		virtual void SetType(std::string type) = 0;
		virtual std::string GetType() = 0;
		virtual void SetValue(std::string value) = 0;
		virtual std::string GetValue() = 0;
	};

	typedef boost::shared_ptr<ContactInfo> ContactInfoPtr;
	typedef std::vector<ContactInfoPtr> ContactInfoList;

	// Status of one individual presence
	// * online status
	// * online message
	class PresenceStatus
	{
	public:
		virtual void SetOnlineStatus(bool status) = 0;
		virtual bool GetOnlineStatus() = 0;
		virtual void SetOnlineMessage(std::string message) = 0;
		virtual std::string GetOnlineMessage() = 0;
	};

	typedef boost::shared_ptr<PresenceStatus> PresenceStatusPtr;

	// Individual contact on contact list 
	// aka. "Friend"
	// * name
	// * presence info
	class Contact
	{
	public:
		virtual void SetName(std::string name) = 0;
		virtual std::string GetName() = 0;
		virtual PresenceStatusPtr GetPresenceStatus() = 0;
		virtual ContactInfoList GetContactInfos() = 0;
	};

	typedef boost::shared_ptr<Contact> ContactPtr;
	typedef std::list<ContactPtr> ContactList;
	typedef boost::shared_ptr<ContactList> ContactListPtr;

	//class ContactList
	//{
	//public:
	//	virtual int GetContactCount() = 0;
	//	virtual int GetOnlineContactCount() = 0;
	//	virtual ContactPtr GetContact() = 0;
	//}

	// A participiant of session
	// * contact
	class Participiant
	{
	public:
		virtual std::string GetName() = 0;
		virtual ContactPtr GetContactData() = 0;
	};

    typedef boost::shared_ptr<Participiant> ParticipiantPtr;

	// * timestamp
	// * author
	// todo: rename to MessageInterface or IMessage ???
	class Message
	{
	public:
		virtual std::string GetTimeStamp() = 0; // todo: change to proper timestamp type
		virtual ParticipiantPtr GetAuthor() = 0; 
		virtual int GetSessionId() = 0;
	private:
	};

	class IMMessage : Message
	{
	public:
		virtual void SetText(std::string text) = 0;
		virtual std::string GetText() = 0;
	};

	typedef boost::shared_ptr<IMMessage> IMMessagePtr;

	// Communication session consisting from participients.
	//
	// Events:
	// - InvitationsResponse(bool)
	// - ParticipientLeft(Participient)
	// - ParticipientJoined(Participient)
	class Session
	{
	public:
		virtual void SendInvitation(ContactPtr c)= 0;
		virtual void Kick(Participiant *p) = 0;
		virtual void Close() = 0;
		virtual int GetId() = 0;
	};

	typedef boost::shared_ptr<Session> SessionPtr;

	class IMSession : Session
	{
	public:
		virtual void SendMessage(IMMessagePtr m) =0 ;
	};

	typedef boost::shared_ptr<IMSession> IMSessionPtr;

	
	// Login information for communication server connection
	class Credentials
	{
		// map of key-value pairs might be better solution ?
		// Todo: settter/getters
	public: // todo: write getters & setters
		std::string type; // eg. "jabber"
		std::string server; // eg. "jabber.org"
		std::string account; // eg. "myaccount@jabber.org"
		std::string password; // eg. "mypassword"
	};

	typedef boost::shared_ptr<Credentials> CredentialsPtr;

	// todo: events:
	// -SessionInvitation
	// -PresenceUpdate
	// -ConnectionStateUpdate
	// -IMMessage
	class CommunicationServiceInterface : public Foundation::ServiceInterface
	{
	public:
		CommunicationServiceInterface() {}
		virtual ~CommunicationServiceInterface() {}

		virtual void OpenConnection(CredentialsPtr c) = 0;
		virtual void CloseConnection() = 0;

		virtual IMSessionPtr CreateIMSession(ContactPtr contact) = 0;
		virtual ContactListPtr GetContactList() = 0;
		virtual void PublishPresence(PresenceStatusPtr p) = 0;
	};

	typedef boost::shared_ptr<CommunicationServiceInterface> CommunicationServicePtr;

} // end of namespace: Communication

#endif // incl_Interfaces_ComminicationServiceInterface_h




