
#include "StableHeaders.h"
#include "Foundation.h"


//#include "EventDataInterface.h"
#include "TelepathyCommunication.h"

namespace Communication
{

	// Split given text with given separator and return substring of given index
	// * Not very optimal, but works
	std::string GetSplitString(std::string text, std::string separator, int index)
	{
		std::string res = "";
		for (int i=0; i<=index; i++)
		{
			int separator_index = text.find(separator);
			if (separator_index == -1)
				if (i == index)
					return text;
				else
					return std::string("");
			res = text.substr(0, separator_index);
			text = text.substr(separator_index + 1, text.size() - separator_index - 1);
			if (i == index)
				return res;
		}
		return "";
	}

	TelepathyCommunicationPtr TelepathyCommunication::instance_;
	void (TelepathyCommunication::*testCallbackPtr)(char*) = NULL;  

	TelepathyCommunication::TelepathyCommunication(Foundation::Framework *f) : connected_(false)
	{
		TelepathyCommunication::instance_ = TelepathyCommunicationPtr(this);
		framework_ = f;

		friend_requests_ = FriendRequestListPtr( new FriendRequestList() );
		im_sessions_ = IMSessionListPtr( new IMSessionList() );
		RegisterConsoleCommands();
		InitializePythonCommunication();
		RegisterEvents();
	}

	TelepathyCommunication::~TelepathyCommunication()
	{
		// todo: Cleanup
	}

	// todo: Add description fields 
	void TelepathyCommunication::RegisterConsoleCommands()
	{
        boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();

        if (console_service)
        {
			console_service->RegisterCommand(Console::CreateCommand("comm", "Prints communication manager debug console commands.", Console::Bind(this, &TelepathyCommunication::ConsoleHelp)));
			console_service->RegisterCommand(Console::CreateCommand("commhelp", "Prints communication manager debug console commands.", Console::Bind(this, &TelepathyCommunication::ConsoleHelp)));
			console_service->RegisterCommand(Console::CreateCommand("commstate", "Information about state of communication manager", Console::Bind(this, &TelepathyCommunication::ConsoleState)));
			console_service->RegisterCommand(Console::CreateCommand("commlogin", "Open connection to IM server. Usage: commlogin(account password server port)", Console::Bind(this, &TelepathyCommunication::ConsoleLogin)));
			console_service->RegisterCommand(Console::CreateCommand("commlogout", "", Console::Bind(this, &TelepathyCommunication::ConsoleLogout)));
			console_service->RegisterCommand(Console::CreateCommand("commcreatesession", "", Console::Bind(this, &TelepathyCommunication::ConsoleCreateSession)));
			console_service->RegisterCommand(Console::CreateCommand("commclosesession", "", Console::Bind(this, &TelepathyCommunication::ConsoleCloseSession)));
			console_service->RegisterCommand(Console::CreateCommand("commlistsessions", "", Console::Bind(this, &TelepathyCommunication::ConsoleListSessions)));
			console_service->RegisterCommand(Console::CreateCommand("commsendmessage", "", Console::Bind(this, &TelepathyCommunication::ConsoleSendMessage)));
			console_service->RegisterCommand(Console::CreateCommand("commlistcontacts", "", Console::Bind(this, &TelepathyCommunication::ConsoleListContacts)));
			console_service->RegisterCommand(Console::CreateCommand("commsendfriendrequest", "", Console::Bind(this, &TelepathyCommunication::ConsoleSendFriendRequest)));
			console_service->RegisterCommand(Console::CreateCommand("commremovecontact", "", Console::Bind(this, &TelepathyCommunication::ConsoleRemoveFriend)));
			console_service->RegisterCommand(Console::CreateCommand("commlistfriendrequests", "", Console::Bind(this, &TelepathyCommunication::ConsoleListFriendRequests)));
			console_service->RegisterCommand(Console::CreateCommand("commaccept", "", Console::Bind(this, &TelepathyCommunication::ConsoleAcceptFriendRequest)));
			console_service->RegisterCommand(Console::CreateCommand("commdeny", "", Console::Bind(this, &TelepathyCommunication::ConsoleDenyFriendRequest)));
		}
	}

	// rename to: InitPythonCommunication
	void TelepathyCommunication::InitializePythonCommunication()
	{
        boost::shared_ptr<Foundation::ScriptServiceInterface> script_service = framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting).lock();
        
		Foundation::ScriptEventInterface* script_event_service = dynamic_cast<Foundation::ScriptEventInterface*>(script_service.get());
		if (script_service == NULL || script_event_service == NULL)
		{
			// todo: handle error
			return;
		}

		std::string error;
		this->communication_py_script_ = Foundation::ScriptObjectPtr(script_service->LoadScript(COMMUNICATION_SCRIPT_NAME, error));
		if(error=="None")
		{
			this->python_communication_object_ = Foundation::ScriptObjectPtr ( this->communication_py_script_->GetObject(COMMUNICATION_CLASS_NAME) );
			std::string name = "CDoStartUp";
			std::string syntax = "";
			char** args = NULL;
			this->python_communication_object_->CallMethod(name, syntax, args); // todo: get return value
		}
		else
		{
			std::string text = "Cannot run python script: ";
			text.append(COMMUNICATION_SCRIPT_NAME);
			LogError(text);
			return;
		}

		// define python callbacks
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackTest, "key");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackConnected, "connected");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackConnecting, "connecting");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackDisconnected, "disconnected");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackChannelOpened, "channel_opened");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackChannelClosed, "channel_closed");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackMessagReceived, "message_received");
		script_event_service->SetCallback( TelepathyCommunication::PycallbackFriendReceived, "contact_item");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackContactStatusChanged, "contact_status_changed");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackMessageSent, "message_sent");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackFriendRequest, "incoming_request");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackContactRemoved, "contact_removed");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackContactAdded, "contact_added");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackFriendRequestLocalPending, "local_pending");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackFriendRequestRemotePending, "remote_pending");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackFriendAdded, "contact_added_publish_list");
		script_event_service->SetCallback( TelepathyCommunication::PyCallbackPresenceStatusTypes, "got_available_status_list");
	}

	void TelepathyCommunication::UninitializePythonCommunication()
	{
		// TODO: free any python related resources
	}

	void TelepathyCommunication::RegisterEvents()
	{
		comm_event_category_ = framework_->GetEventManager()->RegisterEventCategory("Communication");  
		event_manager_ = framework_->GetEventManager();
	}

	/*
	  Currently uses credential from Account.txt
	  todo: use given credential instead
	*/
	void TelepathyCommunication::OpenConnection(CredentialsPtr c)
	{
		if (connected_)
		{
			LogError("Connection to IM server already exist!");
			return;
		}

		std::string method = "CAccountConnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
	}

	/*
	 * Closes previously opened connection to IM server (there is only one connection instance at the time)
	 */
	void TelepathyCommunication::CloseConnection()
	{
		if (!connected_)
		{
			LogError("Connection to IM server doesn't exist!");
			return;
		}

		std::string method = "CDisconnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
	}

	// Not implemented yet: We allow sessions only with contacts at this point of development
	IMSessionPtr TelepathyCommunication::CreateIMSession(ContactInfoPtr contact_info)
	{
		std::string error_message;
		error_message.append("NOT IMPELEMENTED: CreateIMSession(ContactPtr contact)");

		throw error_message;
	}

	/*
	 * Creates a new IM session with given contact.
	 * /todo: in future it's possible to add participients to existing session
	 */
	IMSessionPtr TelepathyCommunication::CreateIMSession(ContactPtr contact)
	{
		std::string protocol = "jabber";

		// Currently we only support jabber
		if ( protocol.compare("jabber") == 0)
		{
//			int session_id = 0; // todo: replace by real id
			TPIMSession* session = new TPIMSession(python_communication_object_);
			session->protocol_ = protocol;
			TPParticipant* participant = new TPParticipant(contact);
			ParticipantPtr participant_ptr = ParticipantPtr((ParticipantInterface*)participant);
			session->participants_->push_back(participant_ptr);

			IMSessionPtr session_ptr = IMSessionPtr((IMSessionInterface*)session);
			this->im_sessions_->push_back(session_ptr);

			char** args = new char*[1];
			char* buf1 = new char[1000];
			std::string address = contact->GetContactInfo(protocol)->GetProperty("address");
			if (address.length() == 0)
			{
				LogError("Given contact has no address");
				// TODO: We must handle error in better way
				return IMSessionPtr((IMSessionInterface*)new TPIMSession(python_communication_object_));
			}
			strcpy(buf1, address.c_str());
			args[0] = buf1;
			std::string method = "CStartChatSession"; // todo: CCreateIMSessionJabber, CCreateIMSessionSIP, etc.
			std::string syntax = "s";
			Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);
			return session_ptr;
			// TODO: Get session id from python ?
		}

		// TODO: Handle error properly
		std::string error_message;
		error_message.append("Protocol [");
		error_message.append(protocol);
		error_message.append("] is not supported by communication manager.");
		LogError(error_message);
		throw error_message;
	}

	// Removes session with given id from im_sessions_ list
	void TelepathyCommunication::RemoveIMSession(std::string session_id)
	{
		for (IMSessionList::iterator i; i < im_sessions_->end(); i++)
		{
			TPIMSession* session =  (TPIMSession*)i->get();
			if ( session->GetId().compare(session_id) == 0)
				im_sessions_->erase(i);
		}
	}

	ContactListPtr TelepathyCommunication::GetContactList()
	{

		ContactList* list = new ContactList();
		for (int i=0; i<contact_list_.size(); i++)
		{
			list->push_back( contact_list_[i]);
		}
		return ContactListPtr(list);
	}

	// static
	TelepathyCommunicationPtr TelepathyCommunication::GetInstance()
	{
		return TelepathyCommunication::instance_;
	}

	void TelepathyCommunication::SetPresenceStatus(PresenceStatusPtr p)
	{
		presence_status_ = p;
		((TPPresenceStatus*)presence_status_.get())->UpdateToServer();
	}

	PresenceStatusPtr TelepathyCommunication::GetPresenceStatus()
	{
		return presence_status_;
	}

	IMMessagePtr TelepathyCommunication::CreateIMMessage(std::string text)
	{
		TPIMMessage* m = new TPIMMessage("");
		m->SetText(text);
		return IMMessagePtr((IMMessageInterface*)m);
	}

	void TelepathyCommunication::SendFriendRequest(ContactInfoPtr contact_info)
	{
		std::string name = "CAddContact";
		std::string syntax = "s";
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, contact_info->GetProperty("address").c_str());
		args[0] = buf1;
		this->python_communication_object_->CallMethod(name, syntax, args); // todo: get return value
	}

	// send remove friend command to IM server
	// todo: remove hardcoded protocol 
	void TelepathyCommunication::RemoveContact(ContactPtr contact)
	{
		std::string name = "CRemoveContact";
		std::string syntax = "s";
		char** args = new char*[1];
		char* buf1 = new char[1000];
		std::string address = contact->GetContactInfo("jabber")->GetProperty("address");
		if ( address.size() == 0)
		{
			std::string text;
			text = "Try to remove nonexist contact (";
			text.append( ((TPContact*)contact.get())->id_ );
			text.append(") from contact list!");
			LogError(text);
			return;
		}
		strcpy(buf1, address.c_str());
		args[0] = buf1;
		this->python_communication_object_->CallMethod(name, syntax, args); // todo: get return value
	}


	ContactPtr TelepathyCommunication::GetContact(std::string id)
	{
		for (ContactList::iterator i = contact_list_.begin(); i < contact_list_.end(); i++)
		{
			ContactPtr c = (*i);
			TPContact* ptr = (TPContact*)c.get();
			if ( ptr->id_.compare(id) == 0)
				return c;
		}

		// ERROR: The given contact does not exist
		std::string error;
		error.append("Requested contact (");
		error.append(id);
		error.append(") wich doesn't exist!");
		LogError(error);
		return ContactPtr((ContactInterface*)new TPContact("")); // todo: handle error better way (we now return empty contact) 
	}

	// DEBUG CONSOLE CALLBACKS:
	// ------------------------

	/*
	 * Shows general information on console about communication manager state 
	 * - connection status
	 * - contacts on contact list (if connected)
	 * - sessions 
     */
	Console::CommandResult TelepathyCommunication::ConsoleState(const Core::StringVector &params)
	{
		char buffer[100];
		std::string text;
		text.append("\n");
		text.append("communication manager: TelepathyCommunication\n");

		text.append("* connection status: ");
		if (connected_)
			text.append("open");
		else
			text.append("closed");
		text.append("\n");
		text.append("* sessions: ");
		sprintf(buffer, "%d",im_sessions_->size());
		text.append("  ");
		text.append(buffer);
		text.append("\n");

		text.append("* contacts: ");
		sprintf(buffer, "%d",contact_list_.size());
		text.append("  ");
		text.append(buffer);
		text.append("\n");

		text.append("* friend requests: ");
		sprintf(buffer, "%d", friend_requests_->size());
		text.append("  ");
		text.append(buffer);
		text.append("\n");
		return Console::ResultSuccess(text); 
	}

	/*
	Prints debug console commands
	*/
	Console::CommandResult TelepathyCommunication::ConsoleHelp(const Core::StringVector &params)
	{
		std::string text;
		text.append("\n");
		text.append("Console commands:\n");
		text.append("comm .................. Prints this help.\n");
		text.append("commhelp .............. Prints this help.\n");
		text.append("commstate ............. Prints information about communication manager state.\n");
		text.append("commlogin ............. Connects to jabber server.\n");
		text.append("commlogout ............ Closes connection to jabber server.\n");
		text.append("commcreatesession ..... Creates IM session.\n");
		text.append("commclosesession ...... Closes IM session.\n");
		text.append("commlistsessions ...... Prints all sessions.\n");
		text.append("commsendmessage ....... Send messaga.\n");
		text.append("commlistcontacts ...... Prints all contacts on contact list.\n");
		text.append("commsendfriendrequest . Sends friend request.\n");
		text.append("commremovecontact ..... Sends friend request.\n");
		text.append("commaccept ............ Accpect friend request.\n");
		text.append("commdeny .............. Deny friend request.\n");
		return Console::ResultSuccess(text); 
	}

	/*
	Opens a connections to jabber server
	*/
	Console::CommandResult TelepathyCommunication::ConsoleLogin(const Core::StringVector &params)
	{
		if ( params.size() != 4 && params.size() != 0 )
		{	
			std::string reason = "Wrong number of arguments!\ncommlogin(<address>,<pwd>,<server>,<port>)";
			return Console::ResultFailure(reason);
		}

		Credentials* c = new Credentials(); // OpenConnection method doesn't use this yet.
		if ( params.size() == 4)
		{
			c->SetProperty("account",params[0]);
			c->SetProperty("password",params[1]);
			c->SetProperty("server",params[2]);
			c->SetProperty("server_port",params[3]);
		}
		OpenConnection( CredentialsPtr(c) );

		std::string text;
		text.append("NOTE: Current version uses credential from Account.txt file.");
		text.append("Ready.");
		return Console::ResultSuccess(text);
	}

	Console::CommandResult TelepathyCommunication::ConsoleLogout(const Core::StringVector &params)
	{
		CloseConnection();
		return Console::ResultSuccess("Ready.");
	}

	// /param params the contact id of contact with we want to start a session
	Console::CommandResult TelepathyCommunication::ConsoleCreateSession(const Core::StringVector &params)
	{
		if ( params.size() != 1 )
		{
			std::string reason = "Wrong number of arguments!\ncommcreatesession(<friend>)";
			return Console::ResultFailure(reason);
		}

		std::string contact_id = params[0];
		ContactPtr c = GetContact(contact_id);

		IMSessionPtr session = CreateIMSession( c );
		((TPIMSession*)session.get())->id_ = "mysession (created from console)";
		// we dont' need our newborn session object nowhere in this case ...

		return Console::ResultSuccess("Ready.");
	}

	Console::CommandResult TelepathyCommunication::ConsoleCloseSession(const Core::StringVector &params)
	{
		if (TelepathyCommunication::GetInstance()->im_sessions_->size() == 0)
		{
			return Console::ResultFailure("There is no session to be closed.");
		}

		((TPIMSession*)(*im_sessions_)[0].get())->Close();

		return Console::ResultSuccess("Ready.");
	}

	// Cannot be implemented before multiple session support is done on python side
	Console::CommandResult TelepathyCommunication::ConsoleListSessions(const Core::StringVector &params)
	{
		std::string text;
		if (im_sessions_->size() == 0)
		{
			text.append("No sessions.");
			return Console::ResultSuccess(text);
		}
		text.append("Sessions: (");
		char buffer[1000]; // TODO: clean this up!
		itoa(im_sessions_->size(), buffer, 10);
		text.append(buffer);
		text.append(")\n");

		ContactInfoPtr jabber_contact;
		for (int i=0; i<im_sessions_->size(); i++)
		{
			IMSessionPtr s = (*im_sessions_)[i];
			std::string id = ((TPIMSession*)s.get())->GetId();
			text.append("* session: ");
			text.append(id);
			text.append("\n");
		}
		text.append("Ready.\n");
		return Console::ResultSuccess(text);
	}

	/*
	 * Send given text to the current IM session (there is only one of them currently)
	 *  todo: support multiple session
	 *  todo: support private messages
	 */	
	Console::CommandResult TelepathyCommunication::ConsoleSendMessage(const Core::StringVector &params)
	{
		// if just one argument then the message is sent to first session we have
		if (params.size() == 1)
		{
			std::string text = params[0];

			if (im_sessions_->size() == 0)
			{
				std::string reason = "There is no session to sent the message!";
				return Console::ResultFailure(reason);
			}

			IMMessagePtr m = CreateIMMessage(text);
			(*im_sessions_).at(0)->SendIMMessage(m);

			return Console::ResultSuccess("Ready.");
		}
		// if we have two arguments then first one is session id and second text
		if (params.size() == 2)
		{
			std::string session_id = params[0];
			std::string text = params[1];

			return Console::ResultFailure("NOT IMPLEMENTED");
		}

		std::string reason = "Wrong number of arguments!\ncommsendmessage(<text>)\ncommsendmessage(<session id>,<text>)";
		return Console::ResultFailure(reason);
	}

	/*
	Prints information to debug console about all contacts in contact list.
	*/
	Console::CommandResult TelepathyCommunication::ConsoleListContacts(const Core::StringVector &params)
	{
		char buffer[100];
		std::string text;

		text.append("Contacts (");
		sprintf(buffer,"%i",contact_list_.size());
		text.append(buffer);
		text.append("):\n");

		for (int i=0; i<contact_list_.size(); i++)
		{
			ContactPtr c = contact_list_[i];
			TPContact* contact = (TPContact*) c.get();
			std::string id = contact->id_;
			std::string name = contact->GetName();
			std::string address = contact->GetContactInfo("jabber")->GetProperty("address");
			std::string online_status = contact->GetPresenceStatus()->GetOnlineStatus();
			std::string online_message = contact->GetPresenceStatus()->GetOnlineMessage();
			text.append( id.append(" ").append(address).append(" ").append(online_status).append(" ").append(online_message) );
		}
		return Console::ResultSuccess(text);
	}

	Console::CommandResult TelepathyCommunication::ConsolePublishPresence(const Core::StringVector &params)
	{
		// todo: call python
		return Console::ResultFailure("NOT IMPLEMENTED.");
	}

	Console::CommandResult TelepathyCommunication::ConsoleSendFriendRequest(const Core::StringVector &params)
	{
		if (params.size() != 1)
		{
			return Console::ResultFailure("Error: Wrong number of parameters. commsendfriendrequest(<address>)");
		}

		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", "jabber");
		info->SetProperty("address", params[0]);
		SendFriendRequest( ContactInfoPtr(info) );
		return Console::ResultSuccess("Ready.");
	}

	// /param params id of friens on friend list
	Console::CommandResult TelepathyCommunication::ConsoleRemoveFriend(const Core::StringVector &params)
	{
		if (params.size() != 1)
		{
			return Console::ResultFailure("Error: Wrong number of parameters. commremovefriend( <id> )");
		}

		std::string id = params[0];
		ContactPtr c = GetContact(id);
		RemoveContact(c);
		return Console::ResultSuccess("Ready.");
	}

	// returns contect of friend request list
	// params Not used
	Console::CommandResult TelepathyCommunication::ConsoleListFriendRequests(const Core::StringVector &params)
	{
		std::string text;
		text = "Friend requests (";
		char buffer[1000];
		itoa(friend_requests_->size(), buffer, 10);
		text.append(buffer);
		text.append("):\n");

		for ( FriendRequestList::iterator i = friend_requests_->begin(); i < friend_requests_->end(); i++)
		{
			FriendRequestPtr r = (*i);
			ContactInfoPtr info = r->GetContactInfo();
			if ( info->GetProperty("protocol").compare("jabber") == 0 )
			{
				std::string address = info->GetProperty("address");
				text.append("* ");
				text.append(address);
				text.append("\n");
			}
		}
		return Console::ResultSuccess(text);
	}

	// params address of friend
	Console::CommandResult TelepathyCommunication::ConsoleAcceptFriendRequest(const Core::StringVector &params)
	{
		if (params.size() != 1)
			return Console::ResultFailure("Wrong number of arguments.\n>> commaccept( <address> )");
		std::string address = params[0];

		for ( FriendRequestList::iterator i = friend_requests_->begin(); i < friend_requests_->end(); i++)
		{
			FriendRequestPtr r = (*i);
			ContactInfoPtr info = r->GetContactInfo();
			if ( info->GetProperty("protocol").compare("jabber") == 0 )
			{
				if ( info->GetProperty("address").compare(address) == 0)
				{
					r->Accept();
					std::string text = "Friend request accepted: ";
					text.append(address);
					return Console::ResultSuccess(text);
				}
			}
		}
		std::string text = "Cannot find friend request: ";
		text.append(address);
		return Console::ResultFailure(text);
	}

	// params address of friend
	Console::CommandResult TelepathyCommunication::ConsoleDenyFriendRequest(const Core::StringVector &params)
	{
		if (params.size() != 1)
			return Console::ResultFailure("Wrong number of arguments.\n>> commadeny( <address> )");
		std::string address = params[0];

		for ( FriendRequestList::iterator i = friend_requests_->begin(); i < friend_requests_->end(); i++)
		{
			FriendRequestPtr r = (*i);
			ContactInfoPtr info = r->GetContactInfo();
			if ( info->GetProperty("protocol").compare("jabber") == 0 )
			{
				if ( info->GetProperty("address").compare(address) == 0)
				{
					r->Deny();
					std::string text = "Friend request accepted: ";
					text.append(address);
					return Console::ResultSuccess(text);
				}
			}
		}
		std::string text = "Cannot find friend request: ";
		text.append(address);
		return Console::ResultFailure(text);
	}

	// PYTHON CALLBACK HANDLERS: 
	// -------------------------

	/*
	   Called by communication.py via PythonScriptModule
	   For internal test from python 
	*/
	void TelepathyCommunication::PyCallbackTest(char* t)
	{
		LogInfo("PyCallbackTest()");
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When request connection is opened
	*/
	void TelepathyCommunication::PyCallbackConnected(char*)
	{
		TelepathyCommunication::GetInstance()->connected_ = true;
		LogInfo("Server connection: Connected");
		// todo : Set own online status to: Online
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When "connecting" message is received from telepathy connection manager
	*/
	void TelepathyCommunication::PyCallbackConnecting(char*)
	{
		TelepathyCommunication::GetInstance()->connected_ = false;
		LogInfo("Server connection: Connecing...");
		// todo : Do we need this? Send notify for UI to be "connecting state"?
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When connection is closed
	*/
	void TelepathyCommunication::PyCallbackDisconnected(char*)
	{
		TelepathyCommunication::GetInstance()->connected_ = false;
		LogInfo("Server connection: Disconnected");
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When new session (telepathy-text-channel) is opened
	   todo: session id should be received (now we don't get anything)
	   todo: This is called both with incoming and outgoing sessions. These should be separedted.
	*/
	void TelepathyCommunication::PyCallbackChannelOpened(char* addr)
	{
		// Find the right session (with participant with given address)
		IMSessionListPtr sessions = TelepathyCommunication::GetInstance()->im_sessions_;
		for (IMSessionList::iterator i = sessions->begin() ; i < sessions->end(); i++)
		{
			IMSessionPtr s = *i;
			ParticipantListPtr participants = ((TPIMSession*)s.get())->GetParticipants();
			for (ParticipantList::iterator j = participants->begin(); j < participants->end(); j++)
			{
				if ( (*j)->GetContact()->GetContactInfo("jabber")->GetProperty("address").compare(addr) == 0)
				{
					// We find the right session and so we can be sure that this session already exist
					// we don't have to do anything
					LogDebug("Channel opened for exist session");
					return;
				}
			}
		}

		// There was no session with this address, we have to create one and send proper event
		// * this happens when session is created by remote partner

		TelepathyCommunicationPtr comm = TelepathyCommunication::GetInstance();
		for (ContactList::iterator i = comm->contact_list_.begin(); i < comm->contact_list_.end(); i++)
		{
			ContactPtr c = *i;
			if ( c->GetContactInfo("jabber")->GetProperty("address").compare(addr) == 0)
			{
				IMSessionPtr s = comm->CreateIMSession(c);
				IMSessionRequestEvent* e = new IMSessionRequestEvent(s);
				IMSessionRequestEventPtr e_ptr = IMSessionRequestEventPtr(e);
				comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::IM_SESSION_REQUEST, (Foundation::EventDataInterface*)&e_ptr);
				std::string text = "Session created for: ";
				text.append(addr);
				LogInfo(text);
				return;
			}
		}

		// The incoming session is from someone who is unknow to us
		// * We do nothin
		// TODO: We can ask user if he wants to allow these messages..
		std::string text = "Session from unknow address: ";
		text.append(addr);
		LogError(text);
		return;
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When (the only) session is closed by remote end
	   // TODO: Session id sould be received 
	   // todo: in future the sessions should not be depended by actual network connection or open channels
	*/
	void TelepathyCommunication::PyCallbackChannelClosed(char*)
	{
		LogInfo("Session closed");
		std::string session_id = "";
		for (int i = 0; i < TelepathyCommunication::GetInstance()->im_sessions_->size(); i++)
		{
			IMSessionPtr s = TelepathyCommunication::GetInstance()->im_sessions_->at(i);
			TPIMSession* im_session = (TPIMSession*)s.get();
			if ( im_session->GetId().compare(session_id) == 0 )
			{
				im_session->NotifyClosedByRemote();
				IMSessionEndEvent e = IMSessionEndEvent(s);
				TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_SESSION_END, (Foundation::EventDataInterface*)&e);
				// todo: remove session from im_sessions_
			}
		}
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When message is received
	   /param text "<address>:<message>"
	*/
	void TelepathyCommunication::PyCallbackMessagReceived(char* address_text)
	{
		std::string address = GetSplitString(address_text,":",0);
		std::string text = GetSplitString(address_text,":",1);
		if (address.length() == 0)
		{
			std::string error;
			error.append("Message received syntax error: ");
			error.append(address_text);
			LogError(error);
			return;
		}

		if (TelepathyCommunication::GetInstance()->im_sessions_->size() == 0)
		{
			std::string t = "";
			t.append("Message received but there is no sessions!");
			LogInfo(t);
			return;
		}

		std::string t;
		t.append("Message received: ");
		t.append(text);
		LogInfo(t);

		IMMessagePtr m = TelepathyCommunication::GetInstance()->CreateIMMessage(text);

		// Find the right session (with given address)
		IMSessionListPtr sessions = TelepathyCommunication::GetInstance()->im_sessions_;
		for (IMSessionList::iterator i = sessions->begin() ; i < sessions->end(); i++)
		{
			IMSessionPtr s = *i;
			ParticipantListPtr participants = ((TPIMSession*)s.get())->GetParticipants();
			for (ParticipantList::iterator j = participants->begin(); j < participants->end(); j++)
			{
				if ( (*j)->GetContact()->GetContactInfo("jabber")->GetProperty("address").compare(address) == 0)
				{
					((TPIMSession*)s.get())->NotifyMessageReceived(m);
					IMMessageEvent e = IMMessageEvent(s,m);
					TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE, (Foundation::EventDataInterface*)&e);
					return;
				}
			}
		}

		// error
		t = "Error: Message from someone out side any sessions";
		LogError(t);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When we get a contact from our contact list from IM server
	   * We build our contact info list here!
	*/
	void TelepathyCommunication::PycallbackFriendReceived(char* id_address)
	{
		LogInfo("PycallbackFriendReceived");

		std::string id = GetSplitString(id_address,":",0);
		std::string address = GetSplitString(id_address,":",1);
		if (id.length() == 0)
		{
			std::string error;
			error.append("Friend act received syntax error: ");
			error.append(id_address);
			LogError(error);
			return;
		}

		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", "jabber");
		info->SetProperty("address", address);

		TPContact* c = new TPContact(id);
		c->SetName(address);
		c->AddContactInfo(ContactInfoPtr(info));
		ContactPtr ptr = ContactPtr( (ContactInterface*)c );
		TelepathyCommunication::GetInstance()->contact_list_.push_back(ptr);
	}


	/*
	   Called by communication.py via PythonScriptModule
	   When presence status has updated
	   /param id <id>:<online status>:<online message>
	*/
	void TelepathyCommunication::PyCallbackContactStatusChanged(char* id_status_message)
	{
		std::string id = GetSplitString(id_status_message, ":", 0);
		std::string status = GetSplitString(id_status_message, ":", 1);
		std::string message = GetSplitString(id_status_message, ":", 2);
		if (id.length() == 0)
		{
			std::string error;
			error.append("Presence update received: Syntax error: ");
			error.append(id_status_message);
			LogError(error);
			return;
		}

		std::string t;
		t.append("Presence changed: ");
		t.append(id);
		t.append("  ");
		t.append(status);
		t.append("  ");
		t.append(message);
		LogInfo(t);

		//ContactList* contact_list = &(TelepathyCommunication::GetInstance()->contact_list_);
		ContactList* contact_list = &TelepathyCommunication::GetInstance()->contact_list_;
		for (int i=0; i<contact_list->size(); i++)
		{
			ContactPtr c = (*contact_list)[i];
			if ( ((TPContact*)c.get())->id_.compare(id) == 0 )
			{
				((TPPresenceStatus*)c->GetPresenceStatus().get())->NotifyUpdate(status, message); 
				PresenceStatusUpdateEvent e = PresenceStatusUpdateEvent( c );
				TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::PRESENCE_STATUS_UPDATE, (Foundation::EventDataInterface*)&e);
			}
		}
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When message was sent 
	*/
	void TelepathyCommunication::PyCallbackMessageSent(char* t)
	{
		LogInfo("PycallbackMessageSent");
		// todo: handle this (maybe IMMessage should have status flag for this ?)
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request (permission to subscripe users presence status) was received
	*/
	// TODO: Remove hard coded "jabber" protocol
	void TelepathyCommunication::PyCallbackFriendRequest(char* address)
	{
		std::string text;
		text.append("PyCallbackFriendRequest: ");
		text.append(address);
		LogInfo("text");

		//ContactInfo* info = new ContactInfo();
		//info->SetProperty("protocol", "jabber");
		//info->SetProperty("address", address);
		//TPFriendRequest* request = new TPFriendRequest( ContactInfoPtr(info) );
		//FriendRequestPtr r = FriendRequestPtr((FriendRequest*)request);
		//TelepathyCommunication::GetInstance()->friend_requests_->push_back(r);
		//FriendRequestEvent* e = new FriendRequestEvent(r);
		//FriendRequestEventPtr e_ptr = FriendRequestEventPtr(e);
		//TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::FRIEND_REQUEST, (Foundation::EventDataInterface*)&e_ptr);
	}


	/*
	   Called by communication.py via PythonScriptModule
	   When contact is removed from contact list
	*/
	void TelepathyCommunication::PyCallbackContactRemoved(char* id)
	{
		TelepathyCommunicationPtr comm = TelepathyCommunication::GetInstance();

		for (ContactList::iterator i = comm->contact_list_.begin(); i < comm->contact_list_.end(); i++)
		{
			TPContact* contact = (TPContact*)((*i).get());
			if (contact->id_.compare(id) == 0)
			{
				comm->contact_list_.erase(i);
				return;
			}
		}

		std::string text;
		text.append("ERROR: Cannot remove contact: ");
		text.append(id);
		LogError(text);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When contact is added to contact list on IM server
	*/
	void TelepathyCommunication::PyCallbackContactAdded(char* id)
	{
		std::string text;
		text.append("PyCallbackContactAdded: ");
		text.append(id);
		LogDebug(text);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request id pending locally
	*/
	void TelepathyCommunication::PyCallbackFriendRequestLocalPending(char* address)
	{
		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", "jabber");
		info->SetProperty("address", address);
		TPFriendRequest* request = new TPFriendRequest( ContactInfoPtr(info) );
		FriendRequestPtr r = FriendRequestPtr((FriendRequestInterface*)request);
		TelepathyCommunication::GetInstance()->friend_requests_->push_back(r);
		FriendRequestEvent* e = new FriendRequestEvent(r);
		FriendRequestEventPtr e_ptr = FriendRequestEventPtr(e);
		TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::FRIEND_REQUEST, (Foundation::EventDataInterface*)&e_ptr);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request id pending on remote side
	*/
	void TelepathyCommunication::PyCallbackFriendRequestRemotePending(char* id)
	{
		// todo: ???
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request id pending on remote side
	   // TODO: This should be renamed better ?
	*/
	void TelepathyCommunication::PyCallbackFriendAdded(char* id)
	{
		// todo: We have to call either of these:
		// 1) Accept
		// 2) send subscription
	}

	/*
	   Called by communication.py via PythonScriptModule
	   After successfully login.

	   /param type_list list of available presence status options (<optio1>:<option2>:<option3>...)
	*/
	void TelepathyCommunication::PyCallbackPresenceStatusTypes(char* type_list)
	{
		std::string option;
		int i = 0;
		do
		{
			option = GetSplitString(type_list, ":", i);
			if ( option.length() > 0 )
			{
				// We don't allow these
				if ( option.compare("unknown") != 0 && option.compare("offline") != 0 )
					TelepathyCommunication::GetInstance()->presence_status_options_.push_back(option);
			}
			i++;
		}
		while (option.length() > 0);
	}

} // end of namespace: Communication
