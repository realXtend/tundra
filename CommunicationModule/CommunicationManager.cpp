
#include "StableHeaders.h"
#include "Foundation.h"


//#include "EventDataInterface.h"
#include "CommunicationManager.h"

namespace Communication
{
	// static
	CommunicationManagerPtr CommunicationManager::instance_;

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
	

//	void (CommunicationManager::*testCallbackPtr)(char*) = NULL;  

	/**
	 * @param framework is pointer to framework. Given here becouse this is not a module class and framework
	 *        services are used through this object.
	 **/
	CommunicationManager::CommunicationManager(Foundation::Framework *framework) : connected_(false)
	{
		CommunicationManager::instance_ = CommunicationManagerPtr(this);
		framework_ = framework;

		friend_requests_ = FriendRequestListPtr( new FriendRequestList() );
		im_sessions_ = IMSessionListPtr( new IMSessionList() );
		presence_status_ = PresenceStatusPtr( (PresenceStatusInterface*) new PresenceStatus() );

		try
		{
			InitializePythonCommunication();
			RegisterConsoleCommands();
			RegisterEvents();
			framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Communication, GetInstance() );
			LogInfo("Service initialized");
		}
		catch(std::string error)
		{
			std::string text = "Cannot start communication service: ";
			text.append(error);
			LogError(text);
		}
	}

	CommunicationManager::~CommunicationManager()
	{
		// todo: unregister service
		// todo: unregister console commands
		UninitializePythonCommunication();
	}

	/**
	 * Define console commands here
	 * @todo Create separate class for console commands
	 * @todo Add description fields 
	 */
	void CommunicationManager::RegisterConsoleCommands()
	{
        boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();

        if (console_service)
        {
			console_service->RegisterCommand(Console::CreateCommand("comm", "Prints communication manager debug console commands.", Console::Bind(this, &CommunicationManager::ConsoleHelp)));
			console_service->RegisterCommand(Console::CreateCommand("commhelp", "Prints communication manager debug console commands.", Console::Bind(this, &CommunicationManager::ConsoleHelp)));
			console_service->RegisterCommand(Console::CreateCommand("commstate", "Information about state of communication manager", Console::Bind(this, &CommunicationManager::ConsoleState)));
			console_service->RegisterCommand(Console::CreateCommand("commlogin", "Open connection to IM server. Usage: commlogin(account password server port)", Console::Bind(this, &CommunicationManager::ConsoleLogin)));
			console_service->RegisterCommand(Console::CreateCommand("commlogout", "", Console::Bind(this, &CommunicationManager::ConsoleLogout)));
			console_service->RegisterCommand(Console::CreateCommand("commcreatesession", "", Console::Bind(this, &CommunicationManager::ConsoleCreateSession)));
			console_service->RegisterCommand(Console::CreateCommand("commclosesession", "", Console::Bind(this, &CommunicationManager::ConsoleCloseSession)));
			console_service->RegisterCommand(Console::CreateCommand("commlistsessions", "", Console::Bind(this, &CommunicationManager::ConsoleListSessions)));
			console_service->RegisterCommand(Console::CreateCommand("commsendmessage", "", Console::Bind(this, &CommunicationManager::ConsoleSendMessage)));
			console_service->RegisterCommand(Console::CreateCommand("commlistcontacts", "", Console::Bind(this, &CommunicationManager::ConsoleListContacts)));
			console_service->RegisterCommand(Console::CreateCommand("commsendfriendrequest", "", Console::Bind(this, &CommunicationManager::ConsoleSendFriendRequest)));
			console_service->RegisterCommand(Console::CreateCommand("commremovecontact", "", Console::Bind(this, &CommunicationManager::ConsoleRemoveFriend)));
			console_service->RegisterCommand(Console::CreateCommand("commlistfriendrequests", "", Console::Bind(this, &CommunicationManager::ConsoleListFriendRequests)));
			console_service->RegisterCommand(Console::CreateCommand("commaccept", "", Console::Bind(this, &CommunicationManager::ConsoleAcceptFriendRequest)));
			console_service->RegisterCommand(Console::CreateCommand("commdeny", "", Console::Bind(this, &CommunicationManager::ConsoleDenyFriendRequest)));
		}
	}

	/**
     * Initialize python object as communication backend
	 */
	void CommunicationManager::InitializePythonCommunication()
	{
        boost::shared_ptr<Foundation::ScriptServiceInterface> script_service = framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting).lock();
		if (!script_service)
		{
			std::string error = "Cannot find ScriptService";
			throw error;
		}
		Foundation::ScriptEventInterface* script_event_service = dynamic_cast<Foundation::ScriptEventInterface*>(script_service.get());

		std::string error;
		this->communication_py_script_ = Foundation::ScriptObjectPtr(script_service->LoadScript(COMMUNICATION_PYTHON_MODULE, error));
		if(error=="None")
		{
			this->python_communication_object_ = Foundation::ScriptObjectPtr ( this->communication_py_script_->GetObject(COMMUNICATION_PYTHON_CLASS) );
			CallPythonCommunicationObject("CDoStartUp");
		}
		else
		{
			std::string text = "Cannot run python script: ";
			text.append(COMMUNICATION_PYTHON_MODULE);
			throw text;
		}

		// define python callbacks
		script_event_service->SetCallback( CommunicationManager::PyCallbackTest, "key");
		script_event_service->SetCallback( CommunicationManager::PyCallbackConnected, "connected");
		script_event_service->SetCallback( CommunicationManager::PyCallbackConnecting, "connecting");
		script_event_service->SetCallback( CommunicationManager::PyCallbackDisconnected, "disconnected");
		script_event_service->SetCallback( CommunicationManager::PyCallbackChannelOpened, "channel_opened");
		script_event_service->SetCallback( CommunicationManager::PyCallbackChannelClosed, "channel_closed");
		script_event_service->SetCallback( CommunicationManager::PyCallbackMessagReceived, "message_received");
		script_event_service->SetCallback( CommunicationManager::PycallbackContactReceived, "contact_item");
		script_event_service->SetCallback( CommunicationManager::PyCallbackContactStatusChanged, "contact_status_changed");
		script_event_service->SetCallback( CommunicationManager::PyCallbackMessageSent, "message_sent");
		script_event_service->SetCallback( CommunicationManager::PyCallbackFriendRequest, "incoming_request");
		script_event_service->SetCallback( CommunicationManager::PyCallbackContactRemoved, "contact_removed");
		script_event_service->SetCallback( CommunicationManager::PyCallbackContactAdded, "contact_added");
		script_event_service->SetCallback( CommunicationManager::PyCallbackFriendRequestLocalPending, "local_pending");
		script_event_service->SetCallback( CommunicationManager::PyCallbackFriendRequestRemotePending, "remote_pending");
		script_event_service->SetCallback( CommunicationManager::PyCallbackFriendAdded, "contact_added_publish_list");
		script_event_service->SetCallback( CommunicationManager::PyCallbackPresenceStatusTypes, "got_available_status_list");
	}

	void CommunicationManager::UninitializePythonCommunication()
	{
		// TODO: free any python related resources
	}

	/**
     * Register event cotegory for communication services: Communication
	 */
	void CommunicationManager::RegisterEvents()
	{
		comm_event_category_ = framework_->GetEventManager()->RegisterEventCategory("Communication");  
		event_manager_ = framework_->GetEventManager();
	}

	/**
	 * Currently uses credential from connection.ini
	 * todo: use given credentials instead
	 */
	void CommunicationManager::OpenConnection(CredentialsPtr c)
	{
		if (connected_)
		{
			LogError("Connection to IM server already exist!");
			return;
		}
		Contact* user_contact = new Contact("user");
		user_contact->SetName("You"); 
		ContactInfo* info = new ContactInfo();
		
		info->SetProperty("protocol", c->GetProperty("protocol") );
		info->SetProperty("address", c->GetProperty("address") );
		user_contact->AddContactInfo(ContactInfoPtr(info));
		user_ = ContactPtr((ContactInterface*)user_contact);

		CallPythonCommunicationObject("CAccountConnect");
	}

	/*
	 * Closes previously opened connection to IM server (there is only one connection instance at the time)
	 */
	void CommunicationManager::CloseConnection()
	{
		if (!connected_)
		{
			LogError("Connection to IM server doesn't exist!");
			return;
		}

		CallPythonCommunicationObject("CDisconnect");
	}

	// Not implemented yet: We allow sessions only with contacts at this point of development
	IMSessionPtr CommunicationManager::CreateIMSession(ContactInfoPtr contact_info)
	{
		std::string error_message;
		error_message.append("NOT IMPELEMENTED: CreateIMSession(ContactPtr contact)");

		throw error_message;
	}

	/**
	 *
	 * Creates a new IM session with given contact.
	 * @todo: in future it's possible to add participients to existing session
	 *
	 **/
	IMSessionPtr CommunicationManager::CreateIMSession(ContactPtr contact)
	{
		return CreateIMSession(contact, user_);
	}

	IMSessionPtr CommunicationManager::CreateIMSession(ContactPtr contact, ContactPtr originator)
	{
		std::string protocol = "jabber";

		// Currently we only support jabber
		if ( protocol.compare("jabber") == 0)
		{
			ParticipantPtr partner = ParticipantPtr( new Participant(contact) );
			ParticipantPtr user = ParticipantPtr( new Participant(user_) );

			IMSession* session = NULL;
			if (contact == originator)
				session = new IMSession(partner, python_communication_object_);
			else
				session = new IMSession(user, python_communication_object_);
			session->id_ = contact->GetContactInfo(protocol)->GetProperty("address"); // todo: some another type to session id 
			IMSessionPtr session_ptr = IMSessionPtr((IMSessionInterface*)session);

			session->protocol_ = protocol;
			session->participants_->push_back(partner);
			session->participants_->push_back(user);
			
			this->im_sessions_->push_back(session_ptr);

			std::string address = contact->GetContactInfo(protocol)->GetProperty("address");
			if (address.length() == 0)
			{
				LogError("Given contact has no address");
				return IMSessionPtr();	// TODO: We must handle error in better way
			}

			CallPythonCommunicationObject("CStartChatSession", address);
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


	/**
	 *  Removes session with given id from im_sessions_ list
	 */
	void CommunicationManager::RemoveIMSession(const IMSession* s)
	{
		for (IMSessionList::iterator i = im_sessions_->begin(); i != im_sessions_->end(); ++i)
		{
			if (s == static_cast<IMSession*>((*i).get()))
			{
				im_sessions_->erase(i);
				return;
			}
		}
		// todo: error handling
	}

	/**
	 *
	 **/
	ContactListPtr CommunicationManager::GetContactList() const
	{

		ContactList* list = new ContactList();
		for (int i=0; i<contact_list_.size(); i++)
		{
			list->push_back( contact_list_[i]);
		}
		return ContactListPtr(list);
	}

	/**
	 *  static method to get instance pointer of communication manager.
	 *  Python script callbacks are static methods and need this intance
	 *  pointer.
	 */
	CommunicationManagerPtr CommunicationManager::GetInstance()
	{
		return CommunicationManager::instance_;
	}

	void CommunicationManager::SetPresenceStatus(PresenceStatusPtr p)
	{
		presence_status_ = p;
		((PresenceStatus*)presence_status_.get())->UpdateToServer();
	}

	PresenceStatusPtr CommunicationManager::GetPresenceStatus()
	{
		return presence_status_;
	}

	IMMessagePtr CommunicationManager::CreateIMMessage(std::string text)
	{
		ParticipantPtr p = ParticipantPtr();
		IMMessage* m = new IMMessage(p, text);
		return IMMessagePtr((IMMessageInterface*)m);
	}

	void CommunicationManager::SendFriendRequest(ContactInfoPtr contact_info)
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
	void CommunicationManager::RemoveContact(ContactPtr contact)
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
			text.append( ((Contact*)contact.get())->id_ );
			text.append(") from contact list!");
			LogError(text);
			return;
		}
		strcpy(buf1, address.c_str());
		args[0] = buf1;
		this->python_communication_object_->CallMethod(name, syntax, args); // todo: get return value
	}

	CredentialsPtr CommunicationManager::GetCredentials()
	{
		Credentials* c = new Credentials();
		c->SetProperty("protocol", "jabber");
		c->SetProperty("address","");
		c->SetProperty("password","");
		c->SetProperty("server address","");
		c->SetProperty("server port","");
		return CredentialsPtr(c);
	}

	ContactPtr CommunicationManager::GetContact(std::string id)
	{
		for (ContactList::iterator i = contact_list_.begin(); i < contact_list_.end(); i++)
		{
			ContactPtr c = (*i);
			Contact* ptr = (Contact*)c.get();
			if ( ptr->id_.compare(id) == 0)
				return c;
		}

		// ERROR: The given contact does not exist
		std::string error;
		error.append("Requested contact (");
		error.append(id);
		error.append(") wich doesn't exist!");
		LogError(error);
		return ContactPtr((ContactInterface*)new Contact("")); // todo: handle error better way (we now return empty contact) 
	}

	// DEBUG CONSOLE CALLBACKS:
	// ------------------------

	/*
	 * Shows general information on console about communication manager state 
	 * - connection status
	 * - contacts on contact list (if connected)
	 * - sessions 
     */
	Console::CommandResult CommunicationManager::ConsoleState(const Core::StringVector &params)
	{
		char buffer[100];
		std::string text;
		text.append("\n");
		text.append("communication manager: CommunicationManager\n");

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
	Console::CommandResult CommunicationManager::ConsoleHelp(const Core::StringVector &params)
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
	Console::CommandResult CommunicationManager::ConsoleLogin(const Core::StringVector &params)
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

	Console::CommandResult CommunicationManager::ConsoleLogout(const Core::StringVector &params)
	{
		CloseConnection();
		return Console::ResultSuccess("Ready.");
	}

	// /param params the contact id of contact with we want to start a session
	Console::CommandResult CommunicationManager::ConsoleCreateSession(const Core::StringVector &params)
	{
		if ( params.size() != 1 )
		{
			std::string reason = "Wrong number of arguments!\ncommcreatesession(<friend>)";
			return Console::ResultFailure(reason);
		}

		std::string contact_id = params[0];
		ContactPtr c = GetContact(contact_id);

		IMSessionPtr session = CreateIMSession( c );
		((IMSession*)session.get())->id_ = "mysession (created from console)";
		// we dont' need our newborn session object nowhere in this case ...

		return Console::ResultSuccess("Ready.");
	}

	Console::CommandResult CommunicationManager::ConsoleCloseSession(const Core::StringVector &params)
	{
		if (CommunicationManager::GetInstance()->im_sessions_->size() == 0)
		{
			return Console::ResultFailure("There is no session to be closed.");
		}

		((IMSession*)(*im_sessions_)[0].get())->Close();

		return Console::ResultSuccess("Ready.");
	}

	// Cannot be implemented before multiple session support is done on python side
	Console::CommandResult CommunicationManager::ConsoleListSessions(const Core::StringVector &params)
	{
		std::string text;
		if (im_sessions_->size() == 0)
		{
			text.append("No sessions.");
			return Console::ResultSuccess(text);
		}
		text.append("Sessions: (");
		text.append(Core::ToString<int>(im_sessions_->size()));
		text.append(")\n");

		ContactInfoPtr jabber_contact;
		for (int i=0; i<im_sessions_->size(); i++)
		{
			IMSessionPtr s = (*im_sessions_)[i];
			std::string id = ((IMSession*)s.get())->id_;
			text.append("* session: [");
			text.append(id);
			text.append("]\n");
		}
		text.append("Ready.\n");
		return Console::ResultSuccess(text);
	}

	/*
	 * Send given text to the current IM session (there is only one of them currently)
	 *  todo: support multiple session
	 *  todo: support private messages
	 */	
	Console::CommandResult CommunicationManager::ConsoleSendMessage(const Core::StringVector &params)
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
	Console::CommandResult CommunicationManager::ConsoleListContacts(const Core::StringVector &params)
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
			Contact* contact = (Contact*) c.get();
			std::string id = contact->id_;
			std::string name = contact->GetName();
			std::string address = contact->GetContactInfo("jabber")->GetProperty("address");
			std::string online_status = contact->GetPresenceStatus()->GetOnlineStatus();
			std::string online_message = contact->GetPresenceStatus()->GetOnlineMessage();
			text.append( id.append(" ").append(address).append(" ").append(online_status).append(" ").append(online_message).append("\n") );
		}
		return Console::ResultSuccess(text);
	}

	Console::CommandResult CommunicationManager::ConsolePublishPresence(const Core::StringVector &params)
	{
		// todo: call python
		return Console::ResultFailure("NOT IMPLEMENTED.");
	}

	Console::CommandResult CommunicationManager::ConsoleSendFriendRequest(const Core::StringVector &params)
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
	Console::CommandResult CommunicationManager::ConsoleRemoveFriend(const Core::StringVector &params)
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
	Console::CommandResult CommunicationManager::ConsoleListFriendRequests(const Core::StringVector &params)
	{
		std::string text;
		text = "Friend requests (";
		text.append(Core::ToString<int>(friend_requests_->size()));
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
	Console::CommandResult CommunicationManager::ConsoleAcceptFriendRequest(const Core::StringVector &params)
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
	Console::CommandResult CommunicationManager::ConsoleDenyFriendRequest(const Core::StringVector &params)
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
	void CommunicationManager::PyCallbackTest(char* t)
	{
		LogInfo("PyCallbackTest()");
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When request connection to IM server was opened
	*/
	void CommunicationManager::PyCallbackConnected(char*)
	{
		CommunicationManager::GetInstance()->connected_ = true;
		LogInfo("Server connection: Connected");
		

		CommunicationManagerPtr comm = CommunicationManager::GetInstance();
		comm->presence_status_->SetOnlineStatus("online");

		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_OPEN);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When "connecting" message is received from telepathy connection manager
	*/
	void CommunicationManager::PyCallbackConnecting(char*)
	{
		CommunicationManager::GetInstance()->connected_ = false;
		LogInfo("Server connection: Connecting...");
		CommunicationManagerPtr comm = CommunicationManager::GetInstance();
        ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_CONNECTING);
        comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);		
		// todo : Do we need this? Send notify for UI to be "connecting state"?
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When connection to IM server was closed
	*/
	void CommunicationManager::PyCallbackDisconnected(char*)
	{
		CommunicationManager::GetInstance()->connected_ = false;
		LogInfo("Server connection: Disconnected");

		CommunicationManagerPtr comm = CommunicationManager::GetInstance();

		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_CLOSE);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When new session (telepathy-text-channel) is opened
	   todo: session id of somekind should be received (we get jabber address)
	   todo: This is called both with incoming and outgoing sessions. These should be separated.
	*/
	void CommunicationManager::PyCallbackChannelOpened(char* addr)
	{
		// Find the right session (with participant with given address, this is the only way currently...)
		IMSessionListPtr sessions = CommunicationManager::GetInstance()->im_sessions_;
		for (IMSessionList::iterator i = sessions->begin() ; i < sessions->end(); i++)
		{
			IMSessionPtr s = *i;
			ParticipantListPtr participants = ((IMSession*)s.get())->GetParticipants();
			for (ParticipantList::iterator j = participants->begin(); j < participants->end(); j++)
			{
				if ( (*j)->GetContact()->GetContactInfo("jabber")->GetProperty("address").compare(addr) == 0)
				{
					// We found the right session so we can be sure that session already exist
					// we don't have to do anything here
					LogDebug("Channel opened for exist session");
					return;
				}
			}
		}

		// There was no session with given address, we have to create one and send proper event
		// * this happens when session is created by remote partner

		CommunicationManagerPtr comm = CommunicationManager::GetInstance();
		for (ContactList::iterator i = comm->contact_list_.begin(); i < comm->contact_list_.end(); i++)
		{
			ContactPtr c = *i;
			if ( c->GetContactInfo("jabber")->GetProperty("address").compare(addr) == 0)
			{
				IMSessionPtr s = comm->CreateIMSession(c, c);
				IMSessionRequestEvent e = IMSessionRequestEvent(s, c);
				comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::IM_SESSION_REQUEST, (Foundation::EventDataInterface*)&e);
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
	void CommunicationManager::PyCallbackChannelClosed(char* address)
	{
		LogInfo("Session closed");
		std::string session_id = address;
		for (int i = 0; i < CommunicationManager::GetInstance()->im_sessions_->size(); i++)
		{
			IMSessionPtr s = CommunicationManager::GetInstance()->im_sessions_->at(i);
			IMSession* im_session = (IMSession*)s.get();
			if ( im_session->id_.compare(session_id) == 0 )
			{
				im_session->NotifyClosedByRemote();
				SessionStateEvent e = SessionStateEvent(s,Events::SessionStateEventInterface::SESSION_END);
				CommunicationManager::GetInstance()->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::SESSION_STATE, (Foundation::EventDataInterface*)&e);

				// todo: remove session from im_sessions_
			}
		}
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When message is received
	   /param text "<address>:<message>"
	*/
	void CommunicationManager::PyCallbackMessagReceived(char* address_text)
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

		if (CommunicationManager::GetInstance()->im_sessions_->size() == 0)
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

		IMMessagePtr m = CommunicationManager::GetInstance()->CreateIMMessage(text);

		// Find the right session (with given address)
		IMSessionListPtr sessions = CommunicationManager::GetInstance()->im_sessions_;
		for (IMSessionList::iterator i = sessions->begin() ; i < sessions->end(); i++)
		{
			IMSessionPtr s = *i;
			ParticipantListPtr participants = ((IMSession*)s.get())->GetParticipants();
			for (ParticipantList::iterator j = participants->begin(); j < participants->end(); j++)
			{
				if ( (*j)->GetContact()->GetContactInfo("jabber")->GetProperty("address").compare(address) == 0)
				{
					// Found the author of message
					((IMMessage*)m.get())->author_ = *j;

					((IMSession*)s.get())->NotifyMessageReceived(m);
					IMMessageEvent e = IMMessageEvent(s,m);
					CommunicationManager::GetInstance()->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE, (Foundation::EventDataInterface*)&e);
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
	   * Send CONNECTION_STATE - CONNECTION_STATE_UPDATE event becouse we have now new information from IM server
	*/
	void CommunicationManager::PycallbackContactReceived(char* id_address)
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

		Contact* c = new Contact(id);
		c->SetName(address);
		c->AddContactInfo(ContactInfoPtr(info));
		ContactPtr ptr = ContactPtr( (ContactInterface*)c );
		CommunicationManager::GetInstance()->contact_list_.push_back(ptr);

		CommunicationManagerPtr comm = CommunicationManager::GetInstance();
		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_STATE_UPDATE);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);

		comm->RequestPresenceStatuses();
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When presence status has updated
	   /param id <id>:<online status>:<online message>
	*/
	void CommunicationManager::PyCallbackContactStatusChanged(char* id_status_message)
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

		//ContactList* contact_list = &(CommunicationManager::GetInstance()->contact_list_);
		ContactList* contact_list = &CommunicationManager::GetInstance()->contact_list_;
		for (int i=0; i<contact_list->size(); i++)
		{
			ContactPtr c = (*contact_list)[i];
			if ( ((Contact*)c.get())->id_.compare(id) == 0 )
			{
				((PresenceStatus*)c->GetPresenceStatus().get())->NotifyUpdate(status, message); 
				PresenceStatusUpdateEvent e = PresenceStatusUpdateEvent( c );
				CommunicationManager::GetInstance()->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::PRESENCE_STATUS_UPDATE, (Foundation::EventDataInterface*)&e);
			}
		}
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When message was sent 
	*/
	void CommunicationManager::PyCallbackMessageSent(char* t)
	{
		LogInfo("PycallbackMessageSent");
		// todo: handle this (maybe IMMessage should have status flag for this ?)
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request (permission to subscripe users presence status) was received
	*/
	// TODO: Remove hard coded "jabber" protocol
	void CommunicationManager::PyCallbackFriendRequest(char* address)
	{
		std::string text;
		text.append("PyCallbackFriendRequest: ");
		text.append(address);
		LogInfo("text");

		//ContactInfo* info = new ContactInfo();
		//info->SetProperty("protocol", "jabber");
		//info->SetProperty("address", address);
		//FriendRequest* request = new FriendRequest( ContactInfoPtr(info) );
		//FriendRequestPtr r = FriendRequestPtr((FriendRequest*)request);
		//CommunicationManager::GetInstance()->friend_requests_->push_back(r);
		//FriendRequestEvent* e = new FriendRequestEvent(r);
		//FriendRequestEventPtr e_ptr = FriendRequestEventPtr(e);
		//CommunicationManager::GetInstance()->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::FRIEND_REQUEST, (Foundation::EventDataInterface*)&e_ptr);
	}


	/**
	 *  Called by communication.py via PythonScriptModule
	 *  When contact is removed from contact list.
	 *  This also happens when user send a friend request and get negative answer
     *  Removes given contact from contact_list_ and send CONNECTION_STATE event
	 *
	 *  @todo for some reason we got this message twise, this should be fixed
 	 */
	void CommunicationManager::PyCallbackContactRemoved(char* id)
	{
		CommunicationManagerPtr comm = CommunicationManager::GetInstance();

		for (ContactList::iterator i = comm->contact_list_.begin(); i < comm->contact_list_.end(); i++)
		{
			Contact* contact = (Contact*)((*i).get());
			if (contact->id_.compare(id) == 0)
			{
				comm->contact_list_.erase(i);

				CommunicationManagerPtr comm = CommunicationManager::GetInstance();
				ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_STATE_UPDATE);
				comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);

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
	   When contact is added to contact list on IM server (friend request is accepted)
	*/
	void CommunicationManager::PyCallbackContactAdded(char* id)
	{
		std::string text;
		text.append("PyCallbackContactAdded: ");
		text.append(id);
		LogDebug(text);

		// TODO: Send ContactListUpdateEvent ?
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request (subscribe request) is received and is waiting for reply

	   Send FriendRequestEvent

	   TODO: if we have request friendship first, we should automatically accept this.
	*/
	void CommunicationManager::PyCallbackFriendRequestLocalPending(char* address)
	{
		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", "jabber");
		info->SetProperty("address", address);
		FriendRequest* request = new FriendRequest( ContactInfoPtr(info) );
		FriendRequestPtr r = FriendRequestPtr((FriendRequestInterface*)request);
		CommunicationManager::GetInstance()->friend_requests_->push_back(r);

		FriendRequestEvent e = FriendRequestEvent(r);
		CommunicationManager::GetInstance()->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::FRIEND_REQUEST, (Foundation::EventDataInterface*)&e);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request id pending on remote side (waiting for accept/deny)
	*/
	void CommunicationManager::PyCallbackFriendRequestRemotePending(char* id)
	{
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When friend request was accpeted by user and now contact wants to subscribe the user

	   We have to subscribe this contact too so we send subsribtion request automatically.
	*/
	void CommunicationManager::PyCallbackFriendAdded(char* id)
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CSendSubscription", id);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   After successfully login.

	   /param type_list list of available presence status options (<optio1>:<option2>:<option3>...)
	*/
	void CommunicationManager::PyCallbackPresenceStatusTypes(char* type_list)
	{
		CommunicationManagerPtr comm = CommunicationManager::GetInstance();
		std::string option;
		int i = 0;
		do
		{
			option = GetSplitString(type_list, ":", i);
			if ( option.length() > 0 )
			{
				// We don't allow these
				if ( option.compare("unknown") != 0 && option.compare("offline") != 0 )
					comm->presence_status_options_.push_back(option);
			}
			i++;
		}
		while (option.length() > 0);

		PresenceStatus::online_status_options_ = comm->presence_status_options_;
		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_STATE_UPDATE);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);
	}

	/**
	 * Request presence status update from IM server
	 */ 
	void CommunicationManager::RequestPresenceStatuses()
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CRefreshContactStatusList");
	}

	/**
	 * Wrapper for calling methods of python object.
     * The object is python_communication_object_ and methos signature is (string method_name, "", NULL)
	 */
	Foundation::ScriptObject* CommunicationManager::CallPythonCommunicationObject(const std::string &method_name) const
	{
		try
		{
			std::string method = method_name;
			std::string syntax = "";
			Foundation::ScriptObject* ret = CommunicationManager::GetInstance()->python_communication_object_->CallMethod(method, syntax, NULL);
			return ret;
		}
		catch(...)
		{
			std::string text;
			text = "Cannot call method [";
			text.append(method_name);
			text.append("] with argument [NULL]");
			LogError(text);
		}
		return NULL; 
	}

	/**
	 * Wrapper for calling methods of python object.
     * The object is python_communication_object_ and methos signature is (string method_name, "s", string arg)
	 */
	Foundation::ScriptObject* CommunicationManager::CallPythonCommunicationObject(const std::string &method_name, const std::string &arg) const
	{
		try
		{
			const int BUFFER_SIZE = 1000;
			char** args = new char*[1];
			char* buf1 = new char[BUFFER_SIZE];
			strcpy(buf1, arg.substr(0,BUFFER_SIZE-1).c_str());
			args[0] = buf1;
			std::string method = method_name;
			std::string syntax = "s";
			Foundation::ScriptObject* ret = CommunicationManager::GetInstance()->python_communication_object_->CallMethod(method, syntax, args);
			return ret;
		}
		catch(...)
		{
			std::string text;
			text = "Cannot call method [";
			text.append(method_name);
			text.append("] with argument [");
			text.append(arg);
			text.append("]");
			LogError(text);
		}
		return NULL; 
	}

} // end of namespace: Communication
