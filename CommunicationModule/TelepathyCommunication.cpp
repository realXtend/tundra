
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
			console_service->RegisterCommand(Console::CreateCommand("commremovefriend", "", Console::Bind(this, &TelepathyCommunication::ConsoleRemoveFriend)));
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
	//		CommunicationModule::LogInfo("ran python script: IMDemo");
			this->python_communication_object_ = Foundation::ScriptObjectPtr ( this->communication_py_script_->GetObject(COMMUNICATION_CLASS_NAME) );
			std::string name = "CDoStartUp";
			std::string syntax = "";
			char** args = NULL;
			this->python_communication_object_->CallMethod(name, syntax, args); // todo: get return value
		}
		else
		{
			// todo: Report about error
	//		CommunicationModule::LogInfo("ERROR: Cannot run python script: IMDemo");
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
			ParticipantPtr participant_ptr = ParticipantPtr((Participant*)participant);
			session->participants_->push_back(participant_ptr);

			IMSessionPtr session_ptr = IMSessionPtr((IMSession*)session);
			this->im_sessions_->push_back(session_ptr);

			char** args = new char*[1];
			char* buf1 = new char[1000];
			strcpy(buf1, contact->GetContactInfo(protocol)->GetProperty("address").c_str());
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

	void TelepathyCommunication::PublishPresence(PresenceStatusPtr p)
	{
		// TODO: call python here
	}

	IMMessagePtr TelepathyCommunication::CreateIMMessage(std::string text)
	{
		TPIMMessage* m = new TPIMMessage("");
		m->SetText(text);
		return IMMessagePtr((IMMessage*)m);
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

	void TelepathyCommunication::RemoveContact(ContactPtr contact)
	{
		std::string name = "CRemoveContact";
		std::string syntax = "s";
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, contact->GetContactInfo("jabber")->GetProperty("address").c_str());
		args[0] = buf1;
		this->python_communication_object_->CallMethod(name, syntax, args); // todo: get return value

		// TODO: Move this to propier python callback function...
		//for (ContactList::iterator i = contact_list_.begin(); i < contact_list_.end(); i++)
		//{
		//	if ( (*i).get() == contact.get() )
		//	{
		//	}
		//}
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
			text.append("online");
		else
			text.append("offline");
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
		text.append("commremovefriend ...... Sends friend request.\n");
		return Console::ResultSuccess(text); 
	}

	/*
	Opens a connections to jabber server
	*/
	Console::CommandResult TelepathyCommunication::ConsoleLogin(const Core::StringVector &params)
	{
		if (params.size() != 4)
		{	
			std::string reason = "Wrong number of arguments!\ncommlogin(<address>,<pwd>,<server>,<port>)";
			return Console::ResultFailure(reason);
		}

		Credentials* c = new Credentials(); // OpenConnection method doesn't use this yet.
		c->SetProperty("account",params[0]);
		c->SetProperty("password",params[1]);
		c->SetProperty("server",params[2]);
		c->SetProperty("server_port",params[3]);
		OpenConnection(CredentialsPtr(c));
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

	Console::CommandResult TelepathyCommunication::ConsoleCreateSession(const Core::StringVector &params)
	{
		if (params.size() != 1)
		{
			std::string reason = "Wrong number of arguments!\ncommcreatesession(<friend>)";
			return Console::ResultFailure(reason);
		}

		ContactInfo* info = new ContactInfo(); 
		info->SetProperty("protocol", "jabber");
		info->SetProperty("address", params[0]);
		ContactInfoPtr info_ptr = ContactInfoPtr(info);

		TPContact* c = new TPContact("test_id");
		c->contact_infos_->push_back( info_ptr );
		ContactPtr c_ptr = ContactPtr((Contact*)c);
		
		IMSessionPtr session = CreateIMSession( c_ptr );
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

	Console::CommandResult TelepathyCommunication::ConsoleRemoveFriend(const Core::StringVector &params)
	{
		if (params.size() != 1)
		{
			return Console::ResultFailure("Error: Wrong number of parameters. commremovefriend(<address>)");
		}

		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", "jabber");
		info->SetProperty("address", params[0]);
		ContactPtr contact = ContactPtr( (Contact*)new TPContact("test2") );
		contact->AddContactInfo(ContactInfoPtr(info));
		RemoveContact(contact);
		return Console::ResultSuccess("Ready.");
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
		// Find the right session (with given address)
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
					return;
				}
			}
		}

		// There was not session with this address, we have to create one
		// * this happens when session is created by remote partner

		ContactInfo* info = new ContactInfo(); 
		info->SetProperty("protocol", "jabber");
		info->SetProperty("address", addr);
		ContactInfoPtr info_ptr = ContactInfoPtr(info);

		TPContact* c = new TPContact("");
		c->contact_infos_->push_back( info_ptr );
		ContactPtr c_ptr = ContactPtr((Contact*)c);
		
		IMSessionPtr session = TelepathyCommunication::GetInstance()->CreateIMSession( c_ptr );
		((TPIMSession*)session.get())->id_ = "mysession (incoming session)";

		LogInfo("Session created");

//		IMSessionPtr s = IMSessionPtr( (IMSession*) new TPIMSession(TelepathyCommunication::GetInstance()->python_communication_object_) );
//		IMSessionInvitationEvent e = IMSessionInvitationEvent(s);
//		TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_SESSION_INVITATION_RECEIVED, (Foundation::EventDataInterface*)&e);
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
		ContactPtr ptr = ContactPtr( (Contact*)c );
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
				TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE, (Foundation::EventDataInterface*)&e);
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


} // end of namespace: Communication
