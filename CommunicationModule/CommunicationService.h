#ifndef incl_CommunicationService_h
#define incl_CommunicationService_h

#include "CoreStdIncludes.h"
#include "ServiceInterface.h"

namespace Communication
{
	// A contact information of individual contact.
	// Eg. jabber id:  "jid": "myjabberid@myjabberprovider.com"
	// 
	class ContactInfo
	{
	public:
		void SetType(std::string type);
		std::string GetType();
		void SetValue(std::string value);
		std::string GetValue();
	private:
		std::string type;
		std::string value;
	};

	typedef std::vector<ContactInfo> ContactInfoList;

	// rename to: PresenceStatus ???
	class PresenceInfo
	{
	public:
		void SetOnlineStatus(bool status);
		bool GetOnlineStatus();
		void SetOnlineMessage(std::string message);
		std::string GetOnlineMessage();
	private:
		bool online_status;
		std::string on_line_message;
	};

	// Individual contact on contact list
	class Contact
	{
	public:
		void SetName(std::string name);
		std::string GetName();
	private:
		std::string name;
//		std::string real_name;
		PresenceInfo presence_info;
		ContactInfoList contact_infos;
	};

	//class ContactGroup
	//{
	//	std::map<std::string, Contact> contacts; // should be map of pointers?
	//};

	// 
	class Participiant
	{
	public:
		std::string GetName();
		Contact GetContactData();

		// Contact* contact
		// communicationCababilities
	};

    typedef boost::shared_ptr<Participiant> ParticipiantPtr;

	// Private Messages
	class Message
	{
	public:
		std::string GetTimeStamp(); // todo: change to timestamp type
		std::string GetAuthor(); // todo: change type to Contact, Particient etc.
		bool IsPrivate();
	private:
	};

	class TextMessage : Message
	{
	public:
		void SetText(std::string text);
		std::string GetText();
	private:
	};

	class PresenceUpdateMessage
	{
		Contact *contact;
	};

	// Communication session consisting from participients.
	//
	// Events:
	// - InvitationsResponse(bool)
	// - ParticipientLeft(Participient)
	// - ParticipientJoined(Participient)
	class ISession
	{
		// messageHistory
		// eventHistory
		void SendInvitation(Participiant *p);
		void Kick(Participiant *p);
		void Close();

		std::list<ParticipiantPtr> GetParticipients();
	};



	// just testing idea
	// events:
	// - Message(IMMessage)
	class IIMSession : ISession
	{
		void SendMessage(Message *m);
		void SendPrivateMessage(Participiant *p, Message *m);
	};

	
	// Login information for communication server connection
	class Credentials
	{
		std::string type; // eg. "jabber"
		std::string server; // eg. "jabber.org"
		std::string account; // eg. "myaccount@jabber.org"
		std::string password; // eg. "mypassword"
	//	std::string protocol; // eg. "XMPP"
	};

	// testing idea...
//	class JabberCredentials : Credentials
//	{
////		credentials_type = "Jabber";
////		std::string jid;
//	};

	// testing idea...
	//class IRCCredentials : Credentials
	//{
	//	std::string nick;
	//	std::string network;
	//};


	// events:
	// -SessionInvitation
	// -PresenceUpdate
	// -ConnectionStateUpdate
	// -Message
	class ICommunicationService : public Foundation::ServiceInterface
	{
	public:
		ICommunicationService() {}
		virtual ~ICommunicationService() {}

		void OpenConnection(Credentials c);
		void CloseAllConnections();
		void OpenSession();
	};
	typedef boost::shared_ptr<ICommunicationService> CommunicationServicePtr;

} // end of namespace: Communication

#endif // incl_CommunicationService_h




