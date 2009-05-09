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

namespace Communication
{
	#define COMMUNICATION_SCRIPT_NAME "IMDemo" // why not IMDemo.py ???
	#define COMMUNICATION_CLASS_NAME "IMDemo"



	class TelepathyCommunication;
	typedef boost::shared_ptr<TelepathyCommunication> TelepathyCommunicationPtr;

	/*
	 * Implements CommunicationServiceInterface with python backend which uses telepathy-python.
	 */
	class TelepathyCommunication : public CommunicationServiceInterface
	{
		friend class TPIMSession;
		friend class TPSession;
		friend class TPFriendRequest;

		MODULE_LOGGING_FUNCTIONS
			
	public:
		TelepathyCommunication(Foundation::Framework *f);
		~TelepathyCommunication(void);

		// CommunicationServiceInterface
		void OpenConnection(CredentialsPtr c);
		void CloseConnection();
		IMSessionPtr CreateIMSession(ContactPtr contact);
		IMSessionPtr CreateIMSession(ContactInfoPtr contact);
		ContactListPtr GetContactList();
		void PublishPresence(PresenceStatusPtr p);
		IMMessagePtr CreateIMMessage(std::string text);
		void SendFriendRequest(ContactInfoPtr contact_info);
		void RemoveContact(ContactPtr contact); // todo: move to ContactList class

		static const std::string NameStatic() { return "ConnectionModule-TelepathyCommunication"; } // for logging functionality

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

		static TelepathyCommunicationPtr GetInstance(); // for python callbacks

	protected:
		static TelepathyCommunicationPtr instance_;

		void RemoveIMSession(std::string session_id); 
		
		// member variables
		bool connected_; // todo: replace this with "Connection" or "ConnectionStatus" object
		Foundation::Framework* framework_;
		IMSessionListPtr im_sessions_;
		ContactList contact_list_;
		Foundation::ScriptObjectPtr communication_py_script_; 
		Foundation::ScriptObjectPtr python_communication_object_; 
		Foundation::EventManagerPtr event_manager_;
		Core::event_category_id_t comm_event_category_; // todo: could be static 
		FriendRequestListPtr friend_requests_;
		

		// python event handlers (todo: could these be non-static member functions?)
		static void PyCallbackTest(char *);
		static void PyCallbackConnected(char*);
		static void PyCallbackConnecting(char*);
		static void PyCallbackDisconnected(char*);
		static void PyCallbackChannelOpened(char*);
		static void PyCallbackChannelClosed(char*);
		static void PyCallbackMessagReceived(char*);
		static void PycallbackFriendReceived(char* t);
		static void PyCallbackContactStatusChanged(char* id);
		static void PyCallbackMessageSent(char* id);
		
		// initialization
		void InitializePythonCommunication();
		void UninitializePythonCommunication();
		void RegisterConsoleCommands();
		void RegisterEvents();
	};

} // end of namespace Communication

#endif // incl_TelepathyCommunication_h
