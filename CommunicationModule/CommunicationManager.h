#ifndef incl_CommunicationManager_h
#define incl_CommunicationManager_h

#include "StableHeaders.h"

#include "Foundation.h"
#include "EventDataInterface.h"

#include "IMMessage.h"
#include "Contact.h"
#include "PresenceStatus.h"
#include "IMSession.h"
#include "CommunicationEvents.h"
#include "Participant.h"
#include "FriendRequest.h"

/**
 * Implementation of CommunicationServiceInterface
 *
 * Uses telepathy-python library as backend using IMDemo python module through PythonScriptModule.
 * Current python implementation uses Telepathy-Gabble connection manager to provice Jabber IM 
 * services. By nature of Telepathy framework it's quite easy to use any connection manager and 
 * the IM services those offers. Communication Service is designed to be generic IM service interface
 * which hide protocol specific features from user.
 *
 * @todo Make this threadsafe
 * @todo paramters are char* in python function calls. Maybe eg. shared_prt or std::string would be better? (But this is python module issue)
 * @todo Move console commands to separate class: ConsoleUI
 * @todo namespace should be renamed to "CommunicationImpl" to hide the implementation from namespace of service interface
 */

namespace Communication
{
	#define COMMUNICATION_PYTHON_MODULE "IMDemo" 
	#define COMMUNICATION_PYTHON_CLASS "IMDemo"

	/**
	 *  Implements CommunicationServiceInterface with python backend which uses telepathy-python
	 */
	class CommunicationManager : public CommunicationServiceInterface
	{
		friend class IMSession;
		friend class Session;
		friend class FriendRequest;
		friend class PresenceStatus;

		MODULE_LOGGING_FUNCTIONS
			
	public:
		CommunicationManager(Foundation::Framework *f);
		~CommunicationManager(void);

		static const std::string NameStatic() { return "CommunicationManager"; } // for logging functionality

		// CommunicationServiceInterface begin
		void OpenConnection(CredentialsPtr c);
		void CloseConnection();
		IMSessionPtr CreateIMSession(ContactPtr contact);
		IMSessionPtr CreateIMSession(ContactInfoPtr contact);
		ContactListPtr GetContactList() const;
		void SetPresenceStatus(PresenceStatusPtr p);
		PresenceStatusPtr GetPresenceStatus();
		IMMessagePtr CreateIMMessage(std::string text);
		void SendFriendRequest(ContactInfoPtr contact_info);
		void RemoveContact(ContactPtr contact); 
		virtual CredentialsPtr GetCredentials(); 
		bool IsInitialized() { return initialized_; }
		
		// CommunicationServiceInterface end

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

	protected:
		static CommunicationManager* GetInstance(); // for python callbacks

		void InitializePythonCommunication();
		void UninitializePythonCommunication();
		void RegisterConsoleCommands();
		void RegisterEvents();

		void RemoveIMSession(const IMSession* s);
		ContactPtr GetContact(std::string id);
		IMSessionPtr CreateIMSession(ContactPtr contact, ContactPtr originator);
		void RequestPresenceStatuses();
		void CallPythonCommunicationObject(const std::string &method_name, const std::string &arg) const;
		void CallPythonCommunicationObject(const std::string &method_name) const;

		// static variables for python callbacks
		static CommunicationManager* instance_;

		// member variables
		bool initialized_; // in such state that the service can be registered
		bool connected_; // \todo replace this with "Connection" or "ConnectionStatus" object
		Foundation::Framework* framework_;
		Foundation::ScriptObjectPtr communication_py_script_; 
		Foundation::ScriptObjectPtr python_communication_object_; 
		Foundation::EventManagerPtr event_manager_;
		Core::event_category_id_t comm_event_category_; // \todo could be static 

		IMSessionListPtr im_sessions_;
		ContactList contact_list_;
		PresenceStatusPtr presence_status_;
		std::vector<std::string> presence_status_options_;
		FriendRequestListPtr friend_requests_;
		ContactPtr user_;
		std::string protocol_;

		// python event handlers
		// \todo: could these be non-static member functions?
		// \todo: could we handle memory allocation in some another way with these function parameters?
		static void PyCallbackTest(char *);
		static void PyCallbackConnected(char*);
		static void PyCallbackConnecting(char*);
		static void PyCallbackDisconnected(char*);
		static void PyCallbackChannelOpened(char*);
		static void PyCallbackChannelClosed(char*);
		static void PyCallbackMessagReceived(char*);
		static void PycallbackContactReceived(char* contact);
		static void PyCallbackPresenceStatusChanged(char* id);
		static void PyCallbackMessageSent(char* id);
		static void PyCallbackFriendRequest(char* id);
		static void PyCallbackContactRemoved(char* id);
		static void PyCallbackContactAdded(char* id);
		static void PyCallbackFriendRequestLocalPending(char* id);
		static void PyCallbackFriendRequestRemotePending(char* id);
		static void PyCallbackFriendAdded(char* id);
		static void PyCallbackPresenceStatusTypes(char* id);
	};

} // end of namespace Communication

#endif // incl_CommunicationManager_h
