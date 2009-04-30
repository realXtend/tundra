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
	class ContactInfo
	{
	public:
		virtual void SetType(std::string type);
		virtual std::string GetType();
		virtual void SetValue(std::string value);
		virtual std::string GetValue();
	private:
		std::string type_;
		std::string value_;
	};

	typedef boost::shared_ptr<ContactInfo> ContactInfoPtr;
	typedef std::vector<ContactInfoPtr> ContactInfoList;

	// Status of one individual presence
	// * online status
	// * online message
	class PresenceStatus
	{
	public:
		virtual void SetOnlineStatus(bool status);
		virtual bool GetOnlineStatus();
		virtual void SetOnlineMessage(std::string message);
		virtual std::string GetOnlineMessage();
	private:
		bool online_status_;
		std::string on_line_message_;
	};

	typedef boost::shared_ptr<PresenceStatus> PresenceStatusPtr;

	// Individual contact on contact list
	// * name
	// * presence info
	class Contact
	{
	public:
		virtual void SetName(std::string name);
		virtual std::string GetName();
		virtual PresenceStatusPtr GetPresenceStatus();
	private:
		std::string name_;
		PresenceStatusPtr presence_status_;
		ContactInfoList contact_infos_;
	};

	typedef boost::shared_ptr<Contact> ContactPtr;

	// A participiant of session
	// * contact
	class Participiant
	{
	public:
		virtual std::string GetName();
		virtual ContactPtr GetContactData();
	private:
		ContactPtr contact_;
	};

    typedef boost::shared_ptr<Participiant> ParticipiantPtr;

	// * timestamp
	// * author
	class Message
	{
	public:
		virtual std::string GetTimeStamp(); // todo: change to proper timestamp type
		virtual ParticipiantPtr GetAuthor(); 
		virtual bool IsPrivate();
	private:
	};

	class IMMessage : Message
	{
	public:
		virtual void SetText(std::string text);
		virtual std::string GetText();
	private:
		std::string text_;
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
	private:
		std::list<ParticipiantPtr> participients_;
	};

	typedef boost::shared_ptr<Session> SessionPtr;

	// todo: events:
	// - Message(IMMessage)
	class IMSession : Session
	{
	public:
		virtual void SendMessage(IMMessagePtr m) =0 ;
//		virtual void SendPrivateMessage(ParticipiantPtr p, IMMessage m);

	private:
		std::list<IMMessagePtr> im_message_history_;
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
		virtual IMSessionPtr CreateIMSession() = 0;
	};

	typedef boost::shared_ptr<CommunicationServiceInterface> CommunicationServicePtr;

} // end of namespace: Communication

#endif // incl_Interfaces_ComminicationServiceInterface_h




