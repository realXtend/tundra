#ifndef incl_TelepathyCommunication_h
#define incl_TelepathyCommunication_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include "TPIMMessage.h"
#include "TPContact.h"
#include "TPPresenceStatus.h"
#include "TPIMSession.h"
#include "CommunicationEvents.h"

// todo: should this be renamed to TPCommunicatation to hide the implmenetation from namespace of interface?
namespace Communication
{
	#define COMMUNICATION_SCRIPT_NAME "IMDemo" // why not IMDemo.py ??? todo: "communication.py" etc.
	#define COMMUNICATION_CLASS_NAME "IMDemo"

	class TelepathyCommunication;
	typedef boost::shared_ptr<TelepathyCommunication> TelepathyCommunicationPtr;

	class TelepathyCommunication : public CommunicationServiceInterface //, public Foundation::ModuleInterfaceImpl
	{
	//	MODULE_LOGGING_FUNCTIONS
	public:
		TelepathyCommunication();
		TelepathyCommunication(Foundation::Framework *f);
		~TelepathyCommunication(void);

		// CommunicationServiceInterface
		void OpenConnection(CredentialsPtr c);
		void CloseConnection();
		IMSessionPtr CreateIMSession(ContactInfoPtr contact);
		ContactListPtr GetContactList();
		void PublishPresence(PresenceStatusPtr p);
		IMMessagePtr CreateIMMessage(std::string text);
		void SendFriendRequest(ContactInfoPtr contact_info);

		// callbacks for console commands
		Console::CommandResult ConsoleInfo(const Core::StringVector &params);
		Console::CommandResult ConsoleHelp(const Core::StringVector &params);
        Console::CommandResult ConsoleLogin(const Core::StringVector &params);
		Console::CommandResult ConsoleLogout(const Core::StringVector &params);
		Console::CommandResult ConsoleCreateSession(const Core::StringVector &params);
		Console::CommandResult ConsoleListSessions(const Core::StringVector &params);
		Console::CommandResult ConsoleSendMessage(const Core::StringVector &params);
		Console::CommandResult ConsoleListContacts(const Core::StringVector &params);
		Console::CommandResult ConsoleAddContact(const Core::StringVector &params);
		Console::CommandResult ConsolePublishPresence(const Core::StringVector &params);


		static TelepathyCommunicationPtr GetInstance(); // for python callbacks

	protected:
		static TelepathyCommunicationPtr instance_;
		
		// member variables
		bool connected_; // todo: replace this with "Connection" object
		Foundation::Framework* framework_;
		TPIMSessionList im_sessions_;
		TPContactList contact_list_;
		Foundation::ScriptObjectPtr communication_py_script_; 
		Foundation::ScriptObjectPtr python_communication_object_; 
		Foundation::EventManagerPtr event_manager_;
		Core::event_category_id_t comm_event_category_; // todo: could be static 
		

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
		
		// initialization
		void InitializePythonCommunication();
		void UninitializePythonCommunication();
		void RegisterConsoleCommands();
		void RegisterEvents();
	};

} // end of namespace Communication

#endif // incl_TelepathyCommunication_h
