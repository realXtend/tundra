#ifndef incl_TelepathyCommunication_h
#define incl_TelepathyCommunication_h


#include "StableHeaders.h"

#include "Foundation.h"
#include "EventDataInterface.h"

#include "TPIMMessage.h"
#include "TPContact.h"
#include "TPPresenceStatus.h"
#include "TPIMSession.h"
#include "CommunicationEvents.h"
#include "TPParticipant.h"
#include "TPFriendRequest.h"

// TODO: Make this threadsafe
// TODO: paramters are char* in python function calls. Maybe eg. shared_prt or std::string would be better? (But this is python module issue)
// TODO: rename all classes to be like: "TPIMSession : IMSession" -> "IMSession : IMSessionInterface"
// todo: should this be renamed to TPCommunicatation to hide the implementation from namespace of service interface
// todo: Call this python function: ("CSendSubscription", "s", user.value);
// TODO: Move console commands to separate class: ConsoleUI

namespace Communication
{
	#define COMMUNICATION_SCRIPT_NAME "IMDemo" // why not IMDemo.py ???
	#define COMMUNICATION_CLASS_NAME "IMDemo"



	class TelepathyCommunication;
	typedef boost::shared_ptr<TelepathyCommunication> TelepathyCommunicationPtr;

	/*
	 * Implements CommunicationServiceInterface with python backend which uses telepathy-python.
	 * @todo rename to CommunicationManger
	 */
	class TelepathyCommunication : public CommunicationServiceInterface
	{
		// TODO: Might be a better way to do communicatio between these classes
		friend class TPIMSession;
		friend class TPSession;
		friend class TPFriendRequest;
		friend class TPPresenceStatus;

		MODULE_LOGGING_FUNCTIONS
			
	public:
		TelepathyCommunication(Foundation::Framework *f);
		~TelepathyCommunication(void);

		// CommunicationServiceInterface
		void OpenConnection(CredentialsPtr c);
		void CloseConnection();
		IMSessionPtr CreateIMSession(ContactPtr contact);
		IMSessionPtr CreateIMSession(ContactInfoPtr contact);
		ContactListPtr GetContactList() const;
		void SetPresenceStatus(PresenceStatusPtr p);
		PresenceStatusPtr GetPresenceStatus();
		IMMessagePtr CreateIMMessage(std::string text);
		void SendFriendRequest(ContactInfoPtr contact_info);
		void RemoveContact(ContactPtr contact); // todo: move to ContactList class

		static const std::string NameStatic() { return "CommunicationManager"; } // for logging functionality

		// callbacks for console commands
		Console::CommandResult ConsoleHelp(const Core::StringVector &params);
		Console::CommandResult ConsoleState(const Core::StringVector &params);
        Console::CommandResult ConsoleLogin(const Core::StringVector &params);
		Console::CommandResult ConsoleLogout(const Core::StringVector &params);
		Console::CommandResult ConsoleCreateSession(const Core::StringVector &params);
		Console::CommandResult ConsoleCloseSession(const Core::StringVector &params);
		Console::CommandResult ConsoleListSessions(const Core::StringVector &params);
		Console::CommandResult ConsoleSendMessage(const Core::StringVector &params);
		Console::CommandResult ConsoleListContacts(const Core::StringVector &params);
		Console::CommandResult ConsolePublishPresence(const Core::StringVector &params);
		Console::CommandResult ConsoleSendFriendRequest(const Core::StringVector &params);
		Console::CommandResult ConsoleRemoveFriend(const Core::StringVector &params);
		Console::CommandResult ConsoleListFriendRequests(const Core::StringVector &params);
		Console::CommandResult ConsoleAcceptFriendRequest(const Core::StringVector &params);
		Console::CommandResult ConsoleDenyFriendRequest(const Core::StringVector &params);

		static TelepathyCommunicationPtr GetInstance(); // for python callbacks

	protected:
		static TelepathyCommunicationPtr instance_;
		void RemoveIMSession(const TPIMSession* s);
		ContactPtr GetContact(std::string id);
		IMSessionPtr CreateIMSession(ContactPtr contact, ContactPtr originator);
		void RequestPresenceStatuses();
		Foundation::ScriptObject* CallPythonCommunicationObject(const std::string &method_name, const std::string &arg) const;
		Foundation::ScriptObject* CallPythonCommunicationObject(const std::string &method_name) const;


		
		
		// member variables
		bool connected_; // todo: replace this with "Connection" or "ConnectionStatus" object
		Foundation::Framework* framework_;
		IMSessionListPtr im_sessions_;
		ContactList contact_list_;
		PresenceStatusPtr presence_status_;
		std::vector<std::string> presence_status_options_;
		FriendRequestListPtr friend_requests_;
		Foundation::ScriptObjectPtr communication_py_script_; 
		Foundation::ScriptObjectPtr python_communication_object_; 
		Foundation::EventManagerPtr event_manager_;
		Core::event_category_id_t comm_event_category_; // todo: could be static 
		ContactPtr user_;

		// python event handlers
		// * todo: could these be non-static member functions?
		// * todo: could we handle memory allocation in some another way with these function parameters?
		static void PyCallbackTest(char *);
		static void PyCallbackConnected(char*);
		static void PyCallbackConnecting(char*);
		static void PyCallbackDisconnected(char*);
		static void PyCallbackChannelOpened(char*);
		static void PyCallbackChannelClosed(char*);
		static void PyCallbackMessagReceived(char*);
		static void PycallbackContactReceived(char* contact);
		static void PyCallbackContactStatusChanged(char* id);
		static void PyCallbackMessageSent(char* id);
		static void PyCallbackFriendRequest(char* id);
		static void PyCallbackContactRemoved(char* id);
		static void PyCallbackContactAdded(char* id);
		static void PyCallbackFriendRequestLocalPending(char* id);
		static void PyCallbackFriendRequestRemotePending(char* id);
		static void PyCallbackFriendAdded(char* id);
		static void PyCallbackPresenceStatusTypes(char* id);

		// initialization
		void InitializePythonCommunication();
		void UninitializePythonCommunication();
		void RegisterConsoleCommands();
		void RegisterEvents();
	};

} // end of namespace Communication

#endif // incl_TelepathyCommunication_h
