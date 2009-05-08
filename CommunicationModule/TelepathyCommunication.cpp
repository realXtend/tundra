
#include "StableHeaders.h"
#include "Foundation.h"


//#include "EventDataInterface.h"
#include "TelepathyCommunication.h"

namespace Communication
{
	using namespace Communication;

	TelepathyCommunicationPtr TelepathyCommunication::instance_;
	void (TelepathyCommunication::*testCallbackPtr)(char*) = NULL;  

	TelepathyCommunication::TelepathyCommunication(Foundation::Framework *f) : connected_(false)
	{
		TelepathyCommunication::instance_ = TelepathyCommunicationPtr(this);
		framework_ = f;

		RegisterConsoleCommands();
		InitializePythonCommunication();
		RegisterEvents();
	}

	TelepathyCommunication::~TelepathyCommunication()
	{
		// todo: Cleanup
	}

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
			console_service->RegisterCommand(Console::CreateCommand("commaddcontact", "", Console::Bind(this, &TelepathyCommunication::ConsoleAddContact)));
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
	  todo: use given credential
	*/
	void TelepathyCommunication::OpenConnection(CredentialsPtr c)
	{
		// todo: check if there is existing connection
		std::string method = "CAccountConnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
	}

	/*
	 * Closes previously opened connection to IM server (there is only one connection instance at the time)
	 */
	void TelepathyCommunication::CloseConnection()
	{
		std::string method = "CDisconnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
	}

	/*
	 * Creates a new IM session with given contact.
	 * /todo: in future it's possible to add participients to existing session
	 */
	IMSessionPtr TelepathyCommunication::CreateIMSession(ContactInfoPtr contact)
	{
		std::string protocol = contact->GetProperty("protocol");
		if (protocol.compare("jabber")==0)
		{
			int session_id = 0; // todo: replace by real id
			TPIMSession* session = new TPIMSession(python_communication_object_);
			IMSessionPtr session_ptr = IMSessionPtr((IMSession*)session);
			this->im_sessions_.push_back(session_ptr);

			char** args = new char*[1];
			char* buf1 = new char[100];
			strcpy(buf1, contact->GetProperty("address").c_str());
			args[0] = buf1;

			std::string method = "CStartChatSession"; // todo: CCreateIMSessionJabber, CCreateIMSessionSIP, etc.
			std::string syntax = "s";
			Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);
			return session_ptr;
			// TODO: Get session id from python !
		}

		// TODO: Handle error properly
		std::string error_message;
		error_message.append("Protocol [");
		error_message.append(protocol);
		error_message.append("] is not supported by communication manager.");

		throw error_message;
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
		// todo: call python here
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
		sprintf(buffer, "%d",im_sessions_.size());
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
		text.append("comm .............. Prints this help.\n");
		text.append("commhelp .......... Prints this help.\n");
		text.append("commstate ......... Prints information about communication manager state.\n");
		text.append("commlogin ......... Connects to jabber server.\n");
		text.append("commlogout ........ Closes connection to jabber server.\n");
		text.append("commcreatesession . Creates IM session.\n");
		text.append("commclosesession .. Closes IM session.\n");
		text.append("commlistsessions .. Prints all sessions.\n");
		text.append("commsendmessage ... Send messaga.\n");
		text.append("commlistcontacts .. Prints all contacts on contact list.\n");
		text.append("commaddcontact .... Sends friend request.\n");
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

		ContactInfo* c = new ContactInfo(); 
		c->SetProperty("protocol", "jabber");
		c->SetProperty("address", params[0]);
		
		IMSessionPtr session = CreateIMSession( ContactInfoPtr(c) );
//		TPIMSession*  s = (TPIMSession*)session.get();
//		s->id_ = "mysession (created from console)";
		((TPIMSession*)session.get())->id_ = "mysession (created from console)";
		// we dont' need our newborn session object nowhere inthis case ...

		return Console::ResultSuccess("Ready.");
	}

	Console::CommandResult TelepathyCommunication::ConsoleCloseSession(const Core::StringVector &params)
	{
		if (TelepathyCommunication::GetInstance()->im_sessions_.size() == 0)
		{
			return Console::ResultFailure("There is no session to be closed.");
		}

		((TPIMSession*)im_sessions_[0].get())->Close();

		return Console::ResultSuccess("Ready.");
	}

	// Cannot be implemented before multiple session support is done on python side
	Console::CommandResult TelepathyCommunication::ConsoleListSessions(const Core::StringVector &params)
	{
		std::string text;
		if (im_sessions_.size() == 0)
		{
			text.append("No sessions.");
			return Console::ResultSuccess(text);
		}
		text.append("Sessions: (");
		char buffer[1000]; // TODO: clean this up!
		itoa(im_sessions_.size(), buffer, 10);
		text.append(buffer);
		text.append(")\n");

		ContactInfoPtr jabber_contact;
		for (int i=0; i<im_sessions_.size(); i++)
		{
			IMSessionPtr s = im_sessions_[i];
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

			if (im_sessions_.size() == 0)
			{
				std::string reason = "There is no session to sent the message!";
				return Console::ResultFailure(reason);
			}

			IMMessagePtr m = CreateIMMessage(text);
			im_sessions_.at(0)->SendIMMessage(m);

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

		text.append("Contacts count: ");
		sprintf(buffer,"%i",contact_list_.size());
		text.append(buffer);
		text.append("\n");

		for (int i=0; i<contact_list_.size(); i++)
		{
			ContactPtr contact = contact_list_[i];
			std::string name = contact->GetName();
			std::string online_status;
			if (contact->GetPresenceStatus()->GetOnlineStatus())
				online_status = "online ";
			else
				online_status = "offline";
			std::string online_message = contact->GetPresenceStatus()->GetOnlineMessage();
			text.append( name.append(" ").append(online_status).append(" ").append(online_message) );
		}
		return Console::ResultSuccess(text);
	}

	Console::CommandResult TelepathyCommunication::ConsoleAddContact(const Core::StringVector &params)
	{
		// todo: call python
		return Console::ResultFailure("NOT IMPLEMENTED.");
	}

	Console::CommandResult TelepathyCommunication::ConsolePublishPresence(const Core::StringVector &params)
	{
		// todo: call python
		return Console::ResultFailure("NOT IMPLEMENTED.");
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
	   When new session was opened
	   todo: session id should be received (now we don't get anything)
	   todo: IS THIS INCOMING OR OUTGOING ???
	*/
	void TelepathyCommunication::PyCallbackChannelOpened(char*)
	{
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
		for (int i = 0; i < TelepathyCommunication::GetInstance()->im_sessions_.size(); i++)
		{
			IMSessionPtr s = TelepathyCommunication::GetInstance()->im_sessions_[i];
			TPIMSession* im_session = (TPIMSession*)s.get();
			if ( im_session->GetId().compare(session_id) == 0 )
			{
				im_session->NotifyClosedByRemote();
				IMSessionClosedEvent e = IMSessionClosedEvent(s);
				TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::SESSION_CLOSED, (Foundation::EventDataInterface*)&e);
				// todo: remove session from im_sessions_
			}
		}
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When message is received
	   todo: session id
	*/
	void TelepathyCommunication::PyCallbackMessagReceived(char* text)
	{
		std::string t;
		t.append("Message received: ");
		t.append(text);
		LogInfo(t);

		if (TelepathyCommunication::GetInstance()->im_sessions_.size() == 0)
		{
			t = "";
			t.append("Message was received but there is no sessions!");
			LogInfo(t);
		}
		std::string session_id = ((TPIMSession*)TelepathyCommunication::GetInstance()->im_sessions_[0].get())->id_; // todo: Get real session id from python
		IMMessagePtr m = TelepathyCommunication::GetInstance()->CreateIMMessage(text);
		//m->SetText(text);
		//IMMessagePtr m = IMMessagePtr((IMMessage*)new TPIMMessage(session_id));
		//m->SetText(text);

		for (int i = 0; i < TelepathyCommunication::GetInstance()->im_sessions_.size(); i++)
		{
			IMSessionPtr s = TelepathyCommunication::GetInstance()->im_sessions_[i];
			if ( ((TPIMSession*)s.get())->GetId().compare(session_id) == 0)
			{
				((TPIMSession*)s.get())->NotifyMessageReceived(m);
			}
		}
		
		IMMessageReceivedEvent e = IMMessageReceivedEvent(m);
		TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE_RECEIVED, (Foundation::EventDataInterface*)&e);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When ???
	*/
	void TelepathyCommunication::PycallbackFriendReceived(char* t)
	{
		LogInfo("PycallbackFriendReceived");
		// todo: handle this
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When presence status has updated
	   param char* id: the id of presence that has updetad
	*/
	void TelepathyCommunication::PyCallbackContactStatusChanged(char* id)
	{
		std::string t;
		t.append("Presence changed: ");
		t.append(id);
		LogInfo(t);

		ContactList* contact_list = &TelepathyCommunication::GetInstance()->contact_list_;
		for (int i=0; i<contact_list->size(); i++)
		{
			ContactPtr c = (*contact_list)[i];
			if (((TPPresenceStatus*)c->GetPresenceStatus().get())->id_.compare(id)==0)
			{
				((TPPresenceStatus*)c->GetPresenceStatus().get())->NotifyUpdate(true, "online message"); 
				// todo: Get the real values to notify with 
				// todo: Call relevant python functions to get actual presence state
				PresenceStatusUpdateEvent e = PresenceStatusUpdateEvent( c );
				TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE_RECEIVED, (Foundation::EventDataInterface*)&e);
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
