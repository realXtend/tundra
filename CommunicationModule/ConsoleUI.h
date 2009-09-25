#ifndef incl_Comm_ConsoleUI_h
#define incl_Comm_ConsoleUI_h

#include "StableHeaders.h"
#include "Foundation.h"
#include "ModuleInterface.h" // for logger

#include "CommunicationManager.h"


namespace CommunicationUI
{

	/**
	 *  Provides console based user interface for communication manager.
	 *  Mainly purpose is to provide tools for testing but it can be used
	 *  for actual comminications too.
	 *
	 */ 
	class ConsoleUI
	{
		MODULE_LOGGING_FUNCTIONS
	public:
		ConsoleUI(Foundation::Framework* framework);
		~ConsoleUI(void);
		static const std::string NameStatic() { return "Communication:ConsoleUI"; } // for logging functionality

	private:
		void PublishConsoleCommands();

		// Console command bindings:

		//! Show general information about CommunicationManager ocject
		Console::CommandResult OnCommandState(const Core::StringVector &params);

		//! Show short help information
		Console::CommandResult OnCommandHelp(const Core::StringVector &params);

		//! Show list of connection in communication manager
		Console::CommandResult OnCommandConnections(const Core::StringVector &params);

		//! Login to IM server (creates an connection object)
		//! params: uid, pwd, server, port
		Console::CommandResult OnCommandLogin(const Core::StringVector &params);

		//! params: connection_id
		Console::CommandResult OnCommandLogout(const Core::StringVector &params);

		//! Show information about current user object 
		Console::CommandResult OnCommandUser(const Core::StringVector &params);

		//! send text message
		//! params: text          send text message to an open text chat session
		//! params: author, text  send text message to given author
		Console::CommandResult OnCommandMessage(const Core::StringVector &params);

		//! Set users presence status
		//! params: state, message
		Console::CommandResult OnCommandSetPresense(const Core::StringVector &params);

		//! Show list of contacts on user's contact list
		//! params: None
		Console::CommandResult OnCommandContacts(const Core::StringVector &params);

		Console::CommandResult OnCommandSendFriendRequest(const Core::StringVector &params);
		Console::CommandResult OnCommandAcceptFriendRequest(const Core::StringVector &params);
		Console::CommandResult OnCommandRejectFriendRequest(const Core::StringVector &params);

		//! Accept all received text chat sessions on queue
		Console::CommandResult OnCommandAcceptChatSession(const Core::StringVector &params);

		//! Close default session
		Console::CommandResult OnCommandClose(const Core::StringVector &params);

		TpQt4Communication::CommunicationManager* communication_manager_;
		Foundation::Framework* framework_;

		//! There can be multiple connection open but this one is used 
		TpQt4Communication::Connection* default_connection_;

		//! There can be multiple chat sessions open but this one is used
		TpQt4Communication::ChatSession* default_chat_session_;
	};

} //end if namespace: CommunicationUI

#endif // incl_Comm_ConsoleUI_h