#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationSettings.h"

#include "CommunicationManager.h"

// Test Telepathy-Farsight
#include <glib.h>
#include <gst/gst.h>
#include <gst/farsight/fs-conference-iface.h>

#include <boost/thread/thread.hpp>
#include <gst/farsight/fs-transmitter.h>
//#include <gst/farsight/fs-session.h>
//#include <gst/farsight/fs-plugin.h>

namespace Communication
{
	// static
	CommunicationManager* CommunicationManager::instance_ = NULL;


	void DBusDaemonThread::operator()()
	{
		char* command = "startdbus.bat";
		int ret = system(command);
	}

	void DBusDaemonThread::Stop()
	{

	}



	typedef struct _TestSession
	{
		FsSession *session;
		FsStream *stream;
	} TestSession;

//#define DEFAULT_AUDIOSRC       "alsasrc"
#define DEFAULT_AUDIOSRC       "dshowaudiosrc" // For windows
#define DEFAULT_AUDIOSINK      "audioconvert ! audioresample ! audioconvert ! alsasink"

	static void src_pad_added_cb (FsStream *stream, GstPad *pad, FsCodec *codec, gpointer user_data)
	{

		// TEST
//		GstElement* alsasrc = gst_element_factory_make("alsasrc")


		GstElement *pipeline = GST_ELEMENT_CAST (user_data);
		GstElement *sink = NULL;
		GError *error = NULL;
		GstPad *pad2;

		//g_print ("Adding receive pipeline\n");

		if (g_getenv ("AUDIOSINK"))
			sink = gst_parse_bin_from_description (g_getenv ("AUDIOSINK"), TRUE, &error);
		else
			sink = gst_parse_bin_from_description (DEFAULT_AUDIOSINK, TRUE, &error);
		//print_error (error);
		g_assert (sink);

		g_assert (gst_bin_add (GST_BIN (pipeline), sink));


		pad2 = gst_element_get_static_pad (sink, "sink");
		g_assert (pad2);

		g_assert (GST_PAD_LINK_SUCCESSFUL (gst_pad_link (pad, pad2)));

		g_assert (gst_element_set_state (sink, GST_STATE_PLAYING) !=
		GST_STATE_CHANGE_FAILURE);

		gst_object_unref (pad2);
	}

	static gboolean async_bus_cb (GstBus *bus, GstMessage *message, gpointer user_data)
	{
		switch (GST_MESSAGE_TYPE(message))
		{
		case GST_MESSAGE_ERROR:
			{
			GError *error = NULL;
			gchar *debug_str = NULL;

			gst_message_parse_error (message, &error, &debug_str);
			g_error ("Got gst message: %s %s", error->message, debug_str);
			}
			break;
		case GST_MESSAGE_WARNING:
			{
			GError *error = NULL;
			gchar *debug_str = NULL;

			gst_message_parse_warning (message, &error, &debug_str);
			g_warning ("Got gst message: %s %s", error->message, debug_str);
			}
			break;
		case GST_MESSAGE_ELEMENT:
			{
			const GstStructure *s = gst_message_get_structure (message);

			if (gst_structure_has_name (s, "farsight-error"))
			{
				gint error;
				const gchar *error_msg = gst_structure_get_string (s, "error-msg");
				const gchar *debug_msg = gst_structure_get_string (s, "debug-msg");

				g_assert (gst_structure_get_enum (s, "error-no", FS_TYPE_ERROR,
                  &error));

				if (FS_ERROR_IS_FATAL (error))
					g_error ("Farsight fatal error: %d %s %s", error, error_msg,
					debug_msg);
				else
					g_warning ("Farsight non-fatal error: %d %s %s", error, error_msg,
		            debug_msg);
			}
			else if (gst_structure_has_name (s, "farsight-new-local-candidate"))
			{
				const GValue *val = gst_structure_get_value (s, "candidate");
				FsCandidate *cand = NULL;

				g_assert (val);
				cand = (FsCandidate*)g_value_get_boxed (val);

				g_print ("New candidate: %s %d\n", cand->ip, cand->port);
			}
			else if (gst_structure_has_name (s,
                "farsight-local-candidates-prepared"))
			{
				g_print ("Local candidates prepared\n");
			}
			else if (gst_structure_has_name (s, "farsight-recv-codecs-changed"))
			{
				const GValue *val = gst_structure_get_value (s, "codecs");
				GList *codecs = NULL;

				g_assert (val);
				codecs = (GList*)g_value_get_boxed (val);

				g_print ("Recv codecs changed:\n");
				for (; codecs; codecs = g_list_next (codecs))
				{
					FsCodec *codec = (FsCodec*)codecs->data;
					gchar *tmp = fs_codec_to_string (codec);
					g_print ("%s\n", tmp);
					g_free (tmp);
				}
			}
			else if (gst_structure_has_name (s, "farsight-send-codec-changed"))
			{
				const GValue *val = gst_structure_get_value (s, "codec");
				FsCodec *codec = NULL;
				gchar *tmp;
				g_assert (val);
				codec = (FsCodec*)g_value_get_boxed (val);
				tmp = fs_codec_to_string (codec);

				g_print ("Send codec changed: %s\n", tmp);
				g_free (tmp);
			}
		}
		break;
		default:
		break;
		}
	
		return TRUE;
	}


	static TestSession* add_audio_session (GstElement *pipeline, FsConference *conf, guint id, FsParticipant *part, guint localport, const gchar *remoteip, guint remoteport)
	{
		
		TestSession *ses = g_slice_new0 (TestSession);
		GError *error = NULL;
		GstPad *pad = NULL, *pad2 = NULL;
		GstElement *src = NULL;
		GList *cands = NULL;
		GList *codecs = NULL;
		GParameter param = {0};
		gboolean res;

		
		ses->session = fs_conference_new_session (conf, FS_MEDIA_TYPE_AUDIO, &error);
//  print_error (error);
		g_assert (ses->session);

		g_object_get (ses->session, "sink-pad", &pad, NULL);

		// Get audio source
		if (g_getenv ("AUDIOSRC"))
			src = gst_parse_bin_from_description (g_getenv ("AUDIOSRC"), TRUE, &error);
		else
			src = gst_parse_bin_from_description (DEFAULT_AUDIOSRC, TRUE, &error);

		// DEBUG
//		src = gst_parse_bin_from_description ("dshowaudiosrc", TRUE, &error);


		//print_error (error);
		g_assert (src);

		g_assert (gst_bin_add (GST_BIN (pipeline), src));

		pad2 = gst_element_get_static_pad (src, "src");
		g_assert (pad2);

		g_assert (GST_PAD_LINK_SUCCESSFUL (gst_pad_link (pad2, pad)));

		gst_object_unref (pad2);
		gst_object_unref (pad);


		cands = g_list_prepend (NULL, fs_candidate_new ("", FS_COMPONENT_RTP,
          FS_CANDIDATE_TYPE_HOST, FS_NETWORK_PROTOCOL_UDP, NULL, localport));

        param.name = "preferred-local-candidates";
		g_value_init (&param.value, FS_TYPE_CANDIDATE_LIST);
		g_value_take_boxed (&param.value, cands);

		gchar** test;
		test = fs_transmitter_list_available();
//		test = fs_plugin_list_available("transmitters");
//		test = fs_session_list_transmitters(ses->session);
		ses->stream = fs_session_new_stream (ses->session, part, FS_DIRECTION_BOTH, "rawudp", 1, &param, &error);
//		ses->stream = fs_session_new_stream (ses->session, part, FS_DIRECTION_BOTH, "rawudp", 0, NULL, &error);
//		ses->stream = fs_session_new_stream (ses->session, part, FS_DIRECTION_BOTH, "gstudp", 1, &param, &error); // TEST
		//print_error (error);
		g_assert (ses->stream);

		g_value_unset (&param.value);

		g_signal_connect (ses->stream, "src-pad-added", G_CALLBACK (src_pad_added_cb), pipeline);

		cands = g_list_prepend (NULL, fs_candidate_new ("", FS_COMPONENT_RTP,
          FS_CANDIDATE_TYPE_HOST, FS_NETWORK_PROTOCOL_UDP, remoteip,  remoteport));

		res = fs_stream_set_remote_candidates (ses->stream, cands, &error);
		//print_error (error);
		g_assert (res);

		fs_candidate_list_destroy (cands);

		codecs = g_list_prepend (NULL, fs_codec_new (FS_CODEC_ID_ANY, "PCMA", FS_MEDIA_TYPE_AUDIO, 0));
		codecs = g_list_prepend (codecs,
		fs_codec_new (FS_CODEC_ID_ANY, "PCMU", FS_MEDIA_TYPE_AUDIO, 0));

		res = fs_session_set_codec_preferences (ses->session, codecs, &error);
		//print_error (error);
		fs_codec_list_destroy (codecs);


		g_object_get (ses->session, "codecs", &codecs, NULL);
		res = fs_stream_set_remote_codecs (ses->stream, codecs, &error);
		//print_error (error);
		g_assert (res);

		return ses;
	}


	//Farsight2Thread::Farsight2Thread()
	//{
	//	int i = 0;
	//}

	//Farsight2Thread::~Farsight2Thread()
	//{}

	/**
	 *  Start gmain loop torun gstream streaming engine
	 */
	void Farsight2Thread::operator()()
	{
		int argc = 4;
		char* argv[4];
		char* temp = "";
		char* local_ip = "5566";
		char* remote_ip = "5567";
		char* remote_address = "192.168.0.201";

		argv[0] = temp;
		argv[1] = local_ip;
		argv[2] = remote_address;
		argv[3] = remote_ip;

//		GOptionGroup* options = gst_init_get_option_group();

//		putenv("GST_PLUGIN_PATH=C:\\gstreamer_bin_test\\lib\\gstreamer-0.10");
		putenv("GST_PLUGIN_PATH=.\\gstreamer-0.10");
			//,C:\\gstreamer_bin_test\\lib\\farsight2-0.0");
		
		gst_init(NULL, NULL);
//		gst_init(&argc, ((char***)&argv));

		// Get GStreamer version
		guint major, minor, micro, nano;
		gst_version (&major, &minor, &micro, &nano);


		GstElement *pipeline = NULL;
		gmain_loop_ = g_main_loop_new (NULL, FALSE);
		
		
	    pipeline = gst_pipeline_new (NULL);
		GstBus *bus = NULL;
		GstElement *conf = NULL;
		FsParticipant *part = NULL;
		GError *error = NULL;


		bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
		gst_bus_add_watch (bus, async_bus_cb, pipeline);
		gst_object_unref (bus);

		conf = gst_element_factory_make ("fsrtpconference", NULL);
		g_assert (conf);

		part = fs_conference_new_participant (FS_CONFERENCE (conf), "test@ignore", &error);
		
	//	LogError(error->message);
		g_assert (part);
		g_assert (gst_bin_add (GST_BIN (pipeline), conf));

	    add_audio_session (pipeline, FS_CONFERENCE (conf), 1, part, 5566, remote_address, 5567);
		g_assert (gst_element_set_state (pipeline, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE);

		g_main_loop_run (gmain_loop_);
		
		

		g_assert (gst_element_set_state (pipeline, GST_STATE_NULL) != GST_STATE_CHANGE_FAILURE);
		g_object_unref (part);
		gst_object_unref (pipeline);
		g_main_loop_unref (gmain_loop_);

	}

	void Farsight2Thread::Stop()
	{
		if (gmain_loop_ == NULL)
		{
			// TODO: RAISE ERROR
			return;
		}

		if (g_main_loop_is_running(gmain_loop_))
			g_main_loop_quit(gmain_loop_);
		
		gst_deinit();
	}

	/**
	 *  Split given text with given separator and return substring of given index
	 */
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
	
		
	void CommunicationManager::StartFarsight2()
	{
		farsight2_thread_ = new Farsight2Thread();
		boost::thread t(*farsight2_thread_);
//		t.join();
	}

	void CommunicationManager::StopFarsight2()
	{
		farsight2_thread_->Stop();
	}

	void CommunicationManager::StartDBusService()
	{
		dbus_daemon_thread_ = new DBusDaemonThread();
		boost::thread t(*dbus_daemon_thread_);
	}




	/**
	 * @param framework is pointer to framework. Given here becouse this is not a module class and framework
	 *        services are used through this object.
	 **/
	CommunicationManager::CommunicationManager(Foundation::Framework *framework) :
		connected_(false),
		initialized_(false),
		python_communication_object_(NULL),
		communication_py_script_(NULL)
	{
		instance_ = this;
		framework_ = framework;

		friend_requests_ = FriendRequestListPtr( new FriendRequestList() );
		im_sessions_ = IMSessionListPtr( new IMSessionList() );
		presence_status_ = PresenceStatusPtr( (PresenceStatusInterface*) new PresenceStatus() );


		// BEGIN of Telepathy-Farsight/GSreamer
		//FsBaseConference conference;


//		Core::Thread my_thread_(&TestThread);
//		boost::thread::id = my_thread.get_id();
		//my_thread_.run();
//		my_thread_.start();

		//StartDBusService();
		StartFarsight2();
		
		
		// END of Telepathy-Farsight/GSreamer 


		try
		{
			InitializePythonCommunication();
			RegisterConsoleCommands();
			RegisterEvents();
            initialized_ = true;
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
		UnInitialize();
	}

	void CommunicationManager::UnInitialize()
	{
		UninitializePythonCommunication();
		instance_ = NULL;	   

		GMainLoop *loop = NULL;
		if (g_main_loop_is_running(loop))
			g_main_loop_quit(loop);
	}

	/**
	 * Define console commands here
	 * @todo Create separate class for console commands
	 * @todo Add description fields for commands
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
	 *
	 * Throws std::string error if cannot initialize python backend
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
		this->communication_py_script_ = script_service->LoadScript(COMMUNICATION_PYTHON_MODULE, error);
		if(error=="None")
		{
			this->python_communication_object_ = this->communication_py_script_->GetObject(COMMUNICATION_PYTHON_CLASS);
			CallPythonCommunicationObject("CDoStartUp");
		}
		else
		{
			std::string text = "Cannot run python script: ";
			text.append(COMMUNICATION_PYTHON_MODULE);
			throw text;
		}

		// define python callbacks
		script_event_service->SetCallback( PyCallbackTest, "key");
		script_event_service->SetCallback( PyCallbackConnected, "connected");
		script_event_service->SetCallback( PyCallbackConnecting, "connecting");
		script_event_service->SetCallback( PyCallbackDisconnected, "disconnected");
		script_event_service->SetCallback( PyCallbackChannelOpened, "channel_opened");
		script_event_service->SetCallback( PyCallbackChannelClosed, "channel_closed");
		script_event_service->SetCallback( PyCallbackMessagReceived, "message_received");
		script_event_service->SetCallback( PycallbackContactReceived, "contact_item");
		script_event_service->SetCallback( PyCallbackPresenceStatusChanged, "contact_status_changed");
		script_event_service->SetCallback( PyCallbackMessageSent, "message_sent");
		script_event_service->SetCallback( PyCallbackFriendRequest, "incoming_request");
		script_event_service->SetCallback( PyCallbackContactRemoved, "contact_removed");
		script_event_service->SetCallback( PyCallbackContactAdded, "contact_added");
		script_event_service->SetCallback( PyCallbackFriendRequestLocalPending, "local_pending");
		script_event_service->SetCallback( PyCallbackFriendRequestRemotePending, "remote_pending");
		script_event_service->SetCallback( PyCallbackFriendAdded, "contact_added_publish_list");
		script_event_service->SetCallback( PyCallbackPresenceStatusTypes, "got_available_status_list");
		script_event_service->SetCallback( PyCallbackAccountCreationSucceeded, "account_registering_succeeded");
        script_event_service->SetCallback( PyCallbackAccountCreationFailed, "account_registering_failed");
	}

	void CommunicationManager::UninitializePythonCommunication()
	{
		if (communication_py_script_)
		{
			delete communication_py_script_;
			communication_py_script_ = NULL;
		}

		if (python_communication_object_)
		{
			delete python_communication_object_;
			python_communication_object_ = NULL;
		}
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
	void CommunicationManager::OpenConnection(CommunicationSettingsInterfacePtr c)
	{
		if (connected_)
		{
			LogError("Connection to IM server already exist!");
			return;
		}
		protocol_ = c->GetProperty("protocol");
		Contact* user_contact = new Contact(LOCAL_USER_ID);
		user_contact->SetName(c->GetProperty("address")); 

		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", protocol_ );
		info->SetProperty("address", c->GetProperty("address") );
		user_contact->AddContactInfo(ContactInfoPtr(info));

		user_ = ContactPtr(static_cast<ContactInterface*>(user_contact));

		CallPythonCommunicationObject("CAccountConnect");
	}

	/**
	 * Closes previously opened connection to IM server (there is only one connection instance at the time)
	 */
	void CommunicationManager::CloseConnection()
	{
		if (!connected_)
		{
			LogError("Connection to IM server doesn't exist!");
			return;
		}
		contact_list_.clear();
		im_sessions_->clear();

		CallPythonCommunicationObject("CDisconnect");
	}
	
	/**
	 * Not implemented yet: We allow sessions only with contacts at this point of development
	 */
	IMSessionPtr CommunicationManager::CreateIMSession(ContactInfoPtr contact_info)
	{
		std::string error_message;
		error_message.append("NOT IMPELEMENTED: CreateIMSession(ContactPtr contact)");

		throw error_message;
	}

	/**
	 * Creates a new IM session with given contact.
	 **/
	IMSessionPtr CommunicationManager::CreateIMSession(ContactPtr contact)
	{
		return CreateIMSession(contact, user_);
	}

	/**
	 *  Creates a IMSession object and add add two participants for it ( given contact and user)
	 */
	IMSessionPtr CommunicationManager::CreateIMSession(ContactPtr contact, ContactPtr originator)
	{
		std::string address = contact->GetContactInfo(protocol_)->GetProperty("address");
		if (address.length() == 0)
		{
			std::string error = "Given contact has no address for protocol";
			error.append(protocol_);
			LogError(error);
			return IMSessionPtr();	// TODO: We must handle error in better way
		}

		ParticipantPtr partner = ParticipantPtr( new Participant(contact) );
		ParticipantPtr user = ParticipantPtr( new Participant(user_) );

		IMSession* session = NULL;
		if (contact == originator)
			session = new IMSession(partner);
		else
			session = new IMSession(user);
		session->id_ = contact->GetContactInfo(protocol_)->GetProperty("address"); // todo: some another type to session id 
		IMSessionPtr session_ptr = IMSessionPtr((IMSessionInterface*)session);

		session->participants_->push_back(partner);
		session->participants_->push_back(user);
		session->user_ = user;
		session->protocol_ = protocol_;
		
		this->im_sessions_->push_back(session_ptr);

		CallPythonCommunicationObject("CStartChatSession", address);
		return session_ptr;
		// TODO: Get session id from python ?
	}


	/**
	 *  Removes given im session from im_sessions_ list
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
	CommunicationManager* CommunicationManager::GetInstance()
	{
		return instance_;
	}

	/**
	 *
	 */
	void CommunicationManager::SetPresenceStatus(PresenceStatusPtr p)
	{
		presence_status_ = p;
		((PresenceStatus*)presence_status_.get())->UpdateToServer();
	}

	/**
	 *
	 */
	PresenceStatusPtr CommunicationManager::GetPresenceStatus()
	{
		return presence_status_;
	}

	/**
	 *  @todo we should add this method to IMSessionInterface too 
	 *        so the messages to be created can be associate to 
	 *        automatically to right session (we can set the 
	 *        author)
	 */
	IMMessagePtr CommunicationManager::CreateIMMessage(std::string text)
	{
		IMMessage* m = new IMMessage(text);
		return IMMessagePtr(static_cast<IMMessageInterface*>(m));
	}

	/**
	 *  Send friend request to IM server
	 */
	void CommunicationManager::SendFriendRequest(ContactInfoPtr contact_info)
	{
		CallPythonCommunicationObject("CAddContact", contact_info->GetProperty("address").c_str());
	}

	/**
	 * send remove friend command to IM server
	 * todo: remove hardcoded protocol 
	 */
	void CommunicationManager::RemoveContact(ContactPtr contact)
	{
		Contact* c = static_cast<Contact*>(contact.get());
		CallPythonCommunicationObject("CRemoveContact", c->GetContactInfo(protocol_)->GetProperty("address"));
	}

	/**
	 *  
	 */
	CommunicationSettingsInterfacePtr CommunicationManager::GetCommunicationSettings()
	{
		// These are fixed attributes for jabber
		// todo: figure out better system
        // todo: this should read the current settings
        CommunicationSettings* commSettings = new CommunicationSettings(this);
        Communication::CommunicationSettingsInterface* commInterface = (CommunicationSettingsInterface*)commSettings ;
        //Communication::CommunicationSettingsInterface* commInterface = new CommunicationSettings();
        CommunicationSettingsInterfacePtr commInterfacePtr = CommunicationSettingsInterfacePtr(commInterface);
        commInterfacePtr->Load();
        return commInterfacePtr;

  //      Credentials* c = new Credentials();
  //      c->Load();
		//c->SetProperty("protocol", "jabber");
		//c->SetProperty("address","");
		//c->SetProperty("password","");
		//c->SetProperty("server address","");
		//c->SetProperty("server port","");
		//return CredentialsPtr(c);
	}

	/**
	 *  Connects to server according to connection settings and creates account and disconnects 
	 */
    void CommunicationManager::CreateAccount()
    {
        this->CallPythonCommunicationObject("CCreateAccount");
    }

	/**
	 *  Find contact from contact list with given contact id
	 *
	 *  @todo Handle the error better
	 */
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
		return ContactPtr(); 
	}

	/**
	 *  Debug console command callacks
	 *  ==============================
	 */

	/**
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

	/**
	 * Prints debug console commands
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

	/**
	 *  Opens a connections to jabber server
	 *  @todo currently IMDemo.py uses the credential from connection.ini 
	 *        we should use these instead
	 */
	Console::CommandResult CommunicationManager::ConsoleLogin(const Core::StringVector &params)
	{
		if ( params.size() != 4 && params.size() != 0 )
		{	
			std::string reason = "Wrong number of arguments!\ncommlogin(<address>,<pwd>,<server>,<port>)";
			return Console::ResultFailure(reason);
		}

		CommunicationSettingsInterface* cs = new CommunicationSettings(this); // OpenConnection method doesn't use this yet.
		if ( params.size() == 4)
		{
			cs->SetProperty("account",params[0]);
			cs->SetProperty("password",params[1]);
			cs->SetProperty("server",params[2]);
			cs->SetProperty("server_port",params[3]);
		}
		OpenConnection( CommunicationSettingsInterfacePtr(cs) );

		std::string text;
		text.append("NOTE: Current version always uses credentials from data/connection.ini.");
		return Console::ResultSuccess(text);
	}

	/**
	 *  Closes the connection to IM server
	 */
	Console::CommandResult CommunicationManager::ConsoleLogout(const Core::StringVector &params)
	{
		CloseConnection();
		return Console::ResultSuccess("Ready.");
	}

	/**
	 * @param params the contact id of contact with we want to start a session
	 */
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

	/**
	 *  Closes the first session in session list
	 *  @todo add parameter for session id
	 */
	Console::CommandResult CommunicationManager::ConsoleCloseSession(const Core::StringVector &params)
	{
		if (CommunicationManager::GetInstance()->im_sessions_->size() == 0)
		{
			return Console::ResultFailure("There is no session to be closed.");
		}

		// TODO: FIX THIS, we close the first session now
		//       We have to close asked session
		((IMSession*)(*im_sessions_)[0].get())->Close();

		return Console::ResultSuccess("Ready.");
	}

	/**
	 *  Prints info about all open sessions
	 */
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

	/**
	 *  Send given text to the current IM session (there is only one of them currently)
	 *  @todo: support multiple session
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

	/**
	 * Prints content of contact list.
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
			std::string address = contact->GetContactInfo(protocol_)->GetProperty("address");
			std::string online_status = contact->GetPresenceStatus()->GetOnlineStatus();
			std::string online_message = contact->GetPresenceStatus()->GetOnlineMessage();
			text.append( id.append(" ").append(address).append(" ").append(online_status).append(" ").append(online_message).append("\n") );
		}
		return Console::ResultSuccess(text);
	}

	/**
	 *  NOT IMPLEMENTED
	 */
	Console::CommandResult CommunicationManager::ConsolePublishPresence(const Core::StringVector &params)
	{
		// todo: call python
		return Console::ResultFailure("NOT IMPLEMENTED.");
	}

	/**
	 *  @params params 1 string: the address of friend to where to send te request
	 */
	Console::CommandResult CommunicationManager::ConsoleSendFriendRequest(const Core::StringVector &params)
	{
		if (params.size() != 1)
		{
			return Console::ResultFailure("Error: Wrong number of parameters. commsendfriendrequest(<address>)");
		}

		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", protocol_);
		info->SetProperty("address", params[0]);
		SendFriendRequest( ContactInfoPtr(info) );
		return Console::ResultSuccess("Ready.");
	}

	/**
	 *  @param params 1 string: the address of contact you want to remove
	 */
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

	/**
	 *  List the open friend requests
	 *  @param params note
	 */
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
			if ( info->GetProperty("protocol").compare(protocol_) == 0 )
			{
				std::string address = info->GetProperty("address");
				text.append("* ");
				text.append(address);
				text.append("\n");
			}
		}
		return Console::ResultSuccess(text);
	}

	/**
	 * @param params 1 string: the address of friend we want to acpect
	 */
	Console::CommandResult CommunicationManager::ConsoleAcceptFriendRequest(const Core::StringVector &params)
	{
		if (params.size() != 1)
			return Console::ResultFailure("Wrong number of arguments.\n>> commaccept( <address> )");
		std::string address = params[0];

		for ( FriendRequestList::iterator i = friend_requests_->begin(); i < friend_requests_->end(); i++)
		{
			FriendRequestPtr r = (*i);
			ContactInfoPtr info = r->GetContactInfo();
			if ( info->GetProperty("protocol").compare(protocol_) == 0 )
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

	/**
	 * @param params 1 string: address of friend which we want to deny
	 */
	Console::CommandResult CommunicationManager::ConsoleDenyFriendRequest(const Core::StringVector &params)
	{
		if (params.size() != 1)
			return Console::ResultFailure("Wrong number of arguments.\n>> commadeny( <address> )");
		std::string address = params[0];

		for ( FriendRequestList::iterator i = friend_requests_->begin(); i < friend_requests_->end(); i++)
		{
			FriendRequestPtr r = (*i);
			ContactInfoPtr info = r->GetContactInfo();
			if ( info->GetProperty("protocol").compare(protocol_) == 0 )
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

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  For internal test from python 
	 */
	void CommunicationManager::PyCallbackTest(char* t)
	{
		LogInfo("PyCallbackTest()");
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When request connection to IM server was opened
	 */
	void CommunicationManager::PyCallbackConnected(char*)
	{
		CommunicationManager::GetInstance()->connected_ = true;
		LogInfo("Server connection: Connected");
		

		CommunicationManager* comm = CommunicationManager::GetInstance();

		// TODO: Fix this, we should get the real status from IM Server
		comm->presence_status_->SetOnlineStatus("available");

		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_OPEN);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);

		comm->RequestPresenceStatuses();
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When "connecting" message is received from telepathy connection manager
	 */
	void CommunicationManager::PyCallbackConnecting(char*)
	{
		CommunicationManager::GetInstance()->connected_ = false;
		LogInfo("Server connection: Connecting...");
		CommunicationManager* comm = CommunicationManager::GetInstance();
        ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_CONNECTING);
        comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);		
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When connection to IM server was closed
	 */
	void CommunicationManager::PyCallbackDisconnected(char*)
	{
		CommunicationManager::GetInstance()->connected_ = false;
		LogInfo("Server connection: Disconnected");

		CommunicationManager* comm = CommunicationManager::GetInstance();

		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_CLOSE);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When new session (telepathy-text-channel) is opened
	 *
	 *  @todo session id of somekind should be received (we get jabber address)
	 *  @todo This is called both with incoming and outgoing sessions. These should be separated.
	 */
	void CommunicationManager::PyCallbackChannelOpened(char* addr)
	{
		CommunicationManager* comm = CommunicationManager::GetInstance();
		// Find the right session (with participant with given address, this is the only way currently...)
		IMSessionListPtr sessions = comm->im_sessions_;
		for (IMSessionList::iterator i = sessions->begin() ; i < sessions->end(); i++)
		{
			IMSessionPtr s = *i;
			ParticipantListPtr participants = ((IMSession*)s.get())->GetParticipants();
			for (ParticipantList::iterator j = participants->begin(); j < participants->end(); j++)
			{
				if ( (*j)->GetContact()->GetContactInfo(comm->protocol_)->GetProperty("address").compare(addr) == 0)
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

		for (ContactList::iterator i = comm->contact_list_.begin(); i < comm->contact_list_.end(); i++)
		{
			ContactPtr c = *i;
			if ( c->GetContactInfo(comm->protocol_)->GetProperty("address").compare(addr) == 0)
			{
				IMSessionPtr s = comm->CreateIMSession(c, c);
				IMSessionRequestEvent e = IMSessionRequestEvent(s, c);
				comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::IM_SESSION_REQUEST, (Foundation::EventDataInterface*)&e);
				std::string text = "Session created for: ";
				text.append(addr);
				LogInfo(text);
				return;
			}
			else
			{
				std::string error = "Cannot craete IM session - unknow originator";
				LogError(error);
			}
		}

		// The incoming session is from someone who is unknow to us
		// * We do nothing
		// TODO: We can ask user if he wants to allow these messages..
		std::string text = "Session from unknow address: ";
		text.append(addr);
		LogError(text);
		return;
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When session is closed 
	 *  @todo in future the sessions should not be depended by actual network connection or open channels
	 *        at least in ui level. So session can be open even the other participant is offline.
	 */
	void CommunicationManager::PyCallbackChannelClosed(char* address)
	{
		CommunicationManager* comm = CommunicationManager::GetInstance();
		LogInfo("Session closed");
		std::string session_id = address;
		for (IMSessionList::iterator i = comm->im_sessions_->begin(); i != comm->im_sessions_->end(); ++i)
		{
			IMSessionPtr s = *i;
			IMSession* im_session = static_cast<IMSession*>(s.get());
			if ( im_session->id_.compare(session_id) == 0 )
			{
				im_session->NotifyClosedByRemote();
				SessionStateEvent e = SessionStateEvent(s,Events::SessionStateEventInterface::SESSION_END);
				CommunicationManager::GetInstance()->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::SESSION_STATE, (Foundation::EventDataInterface*)&e);
			}
		}
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When message is received
	 *  @param address_text format: "<address>:<message>"
	 */
	void CommunicationManager::PyCallbackMessagReceived(char* address_text)
	{
		CommunicationManager* comm = CommunicationManager::GetInstance();

		std::string address = GetSplitString(address_text,":",0);
        //\bug does this work if there's : characters in message?
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
			//\todo Might be a good idea handle also these out of session messages
			//      Perhaps we could create session
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

		// Find the right session (wich includes participant with given address)

		IMSessionListPtr sessions = CommunicationManager::GetInstance()->im_sessions_;
		for (IMSessionList::iterator i = sessions->begin() ; i != sessions->end(); ++i)
		{
			IMSessionPtr s = *i;
			ParticipantListPtr participants = s->GetParticipants();
			for (ParticipantList::iterator j = participants->begin(); j != participants->end(); ++j)
			{
				ParticipantPtr participant = *j;
				if (participant->GetContact()->GetContactInfo(comm->protocol_)->GetProperty("address").compare(address) == 0)
				{
					// Found the author of message
					
					((IMMessage*)m.get())->author_ = participant;
					((IMSession*)s.get())->NotifyMessageReceived(m);

					IMMessageEvent e = IMMessageEvent(s,m);
					comm->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE, (Foundation::EventDataInterface*)&e);
					return;
				}
			}
		}

		// error
		std::string error = "Error: Out of session message received";
		error.append(address_text);
		LogError(error);
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When we get a contact item in our contact list from IM server. 
	 *  We build the contact list here
	 *  \param id_address Contact id and IM address in format: <id>:<address>
	 *
	 *  Events:
	 *  - Send CONNECTION_STATE / CONNECTION_STATE_UPDATE event because we have now new information
	 *    to ui to show
     */
	void CommunicationManager::PycallbackContactReceived(char* id_address)
	{
		CommunicationManager* comm = CommunicationManager::GetInstance();
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
		info->SetProperty("protocol", comm->protocol_);
		info->SetProperty("address", address);

		Contact* c = new Contact(id);
		c->SetName(address);
		c->AddContactInfo(ContactInfoPtr(info));
		ContactPtr ptr = ContactPtr(static_cast<ContactInterface*>(c));
		CommunicationManager::GetInstance()->contact_list_.push_back(ptr);
		
		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_STATE_UPDATE);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);

		comm->RequestPresenceStatuses();
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When presence status has update was received from IM server
	 *  \param id_status_message format: <id>:<online status>:<online message>
	 */
	void CommunicationManager::PyCallbackPresenceStatusChanged(char* id_status_message)
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

		CommunicationManager* comm = CommunicationManager::GetInstance();

		if (id.compare(LOCAL_USER_ID) == 0)
		{
			comm->presence_status_->SetOnlineStatus(status);
			comm->presence_status_->SetOnlineMessage(message);
			return;
		}

		for (ContactList::iterator i = comm->contact_list_.begin(); i != comm->contact_list_.end(); ++i)
		{
			ContactPtr c = *(i);
			Contact* contact = static_cast<Contact*>(c.get());
			if (contact->id_.compare(id) == 0)
			{
				PresenceStatus* s = static_cast<PresenceStatus*>(c->GetPresenceStatus().get());
				s->NotifyUpdate(status, message); 
				PresenceStatusUpdateEvent e = PresenceStatusUpdateEvent(c);
				comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::PRESENCE_STATUS_UPDATE, (Foundation::EventDataInterface*)&e);
				return;
			}
		}

		std::string error = "Presence status update for unknow contact: ";
		error.append(id_status_message);
		LogError(error);
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When message sent ack is received from IM server
	 */
	void CommunicationManager::PyCallbackMessageSent(char* t)
	{
		LogInfo("PycallbackMessageSent");
		//\todo: handle this (maybe IMMessage should have ack status flag?)
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When friend request (permission to subscripe user's presence status) was received
	 *  \param address The IM address of originator of this request.
	 *
	 *  \todo HANDLE THIS
     */
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
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When contact is removed from contact list.
	 *  This also happens when user send a friend request and get deby answer
     *  Removes given contact from contact_list_
	 *  \note Send CONNECTION_STATE event.
	 *  \param id Contact id 
	 *  \todo for some reason we got this message twise, this should be fixed
 	 */
	void CommunicationManager::PyCallbackContactRemoved(char* id)
	{
		CommunicationManager* comm = CommunicationManager::GetInstance();

		for (ContactList::iterator i = comm->contact_list_.begin(); i != comm->contact_list_.end(); ++i)
		{
			Contact* contact = static_cast<Contact*>((*i).get());
			if (contact->id_.compare(id) == 0)
			{
				comm->contact_list_.erase(i);

				CommunicationManager* comm = CommunicationManager::GetInstance();
				ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_STATE_UPDATE);
				comm->event_manager_->SendEvent(comm->comm_event_category_, Communication::Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);

				return;
			}
		}

		std::string error;
		error.append("ERROR: Cannot remove contact, isn't in contact list: "); 
		error.append(id);
		LogError(error);
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When contact is added to contact list on IM server (friend request is accepted by both participants)
	 *  \param id Contact id for this new contact 
	 */
	void CommunicationManager::PyCallbackContactAdded(char* id)
	{
		std::string text;
		text.append("PyCallbackContactAdded: ");
		text.append(id);
		LogDebug(text);

		//\todo: Check that will we get the PyCallbackContactReceived or not!
		//       If not then we have to manually add this contact to our contact list

		CommunicationManager::GetInstance()->RequestPresenceStatuses();
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When friend request (subscribe request) is received and is waiting for reply
     *  \param id ???
	 *  Send FriendRequestEvent
     *
	 *  TODO: if we have request friendship first, we should automatically accept this.
	 */
	void CommunicationManager::PyCallbackFriendRequestLocalPending(char* address)
	{
		CommunicationManager* comm = CommunicationManager::GetInstance();
		ContactInfo* info = new ContactInfo();
		info->SetProperty("protocol", comm->protocol_);
		info->SetProperty("address", address);
		FriendRequest* request = new FriendRequest( ContactInfoPtr(info) );
		FriendRequestPtr r = FriendRequestPtr((FriendRequestInterface*)request);
		CommunicationManager::GetInstance()->friend_requests_->push_back(r);

		FriendRequestEvent e = FriendRequestEvent(r);
		comm->event_manager_->SendEvent(CommunicationManager::GetInstance()->comm_event_category_, Communication::Events::FRIEND_REQUEST, (Foundation::EventDataInterface*)&e);
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When friend request id pending on remote side (waiting for accept/deny) ???
	 *  \note This is not currently used by IMDemo.py
	 *  \todo If this is unnecessary callback we should remove this
	 *  \param id Unknow
	 */
	void CommunicationManager::PyCallbackFriendRequestRemotePending(char* id)
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CAcceptContactRequest", id);
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  When friend request was accpeted by user and now the new friend wants to subscribe the user too
     *  \param address IM address of new friend
	 *  We have already accepted friendship so we send subsribtion request automatically here.
	 */
	void CommunicationManager::PyCallbackFriendAdded(char* address)
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CSendSubscription", address);
	}

	/**
	 *  Called by IMDemo.py via PythonScriptModule
	 *  After successfully login.
	 *  \param type_list List of available presence status options in format: <optio1>:<option2>:<option3>...
	 *  \note We filter some options here because it's not allowed to user set those.
	 *  \note Send CONNECTION_STATE event
	 */
	void CommunicationManager::PyCallbackPresenceStatusTypes(char* type_list)
	{
		CommunicationManager* comm = CommunicationManager::GetInstance();
		std::string option;
		int i = 0;
		do
		{
			option = GetSplitString(type_list, ":", i++);
			if (option.length() > 0)
			{
				if ( option.compare("unknown") == 0 || option.compare("offline") == 0 || option.compare("error") == 0)
					continue;

				comm->presence_status_options_.push_back(option);
			}
		}
		while (option.length() > 0);

		PresenceStatus::online_status_options_ = comm->presence_status_options_;
		ConnectionStateEvent e = ConnectionStateEvent(Events::ConnectionStateEventInterface::CONNECTION_STATE_UPDATE);
		comm->event_manager_->SendEvent(comm->comm_event_category_, Events::CONNECTION_STATE, (Foundation::EventDataInterface*)&e);
	}

    void CommunicationManager::PyCallbackAccountCreationSucceeded(char* reason)
    {
        CommunicationManager* comm = CommunicationManager::GetInstance();
        ConnectionStateEvent e = ConnectionStateEvent(Events::AccountCreationEventInterface::ACCOUNT_CREATION_SUCCEEDED);
        comm->event_manager_->SendEvent(comm->comm_event_category_, Events::ACCOUNT_CREATION, (Foundation::EventDataInterface*)&e);
    }
    void CommunicationManager::PyCallbackAccountCreationFailed(char* reason)
    {
        CommunicationManager* comm = CommunicationManager::GetInstance();
        ConnectionStateEvent e = ConnectionStateEvent(Events::AccountCreationEventInterface::ACCOUNT_CREATION_FAILED);
        comm->event_manager_->SendEvent(comm->comm_event_category_, Events::ACCOUNT_CREATION, (Foundation::EventDataInterface*)&e);
    }

	/**
	 *  Requests presence status updates from IM server
	 */ 
	void CommunicationManager::RequestPresenceStatuses()
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CRefreshContactStatusList");
	}

	/**
	 *  Wrapper for calling methods of python objects.
	 *  \param method_name The method name
     *  \note The calls methods of python_communication_object_ with signature: (string method_name, "", NULL)
	 */
	void CommunicationManager::CallPythonCommunicationObject(const std::string &method_name) const
	{
		try
		{
			std::string method = method_name;
			std::string syntax = "";
			Foundation::ScriptObject* ret = CommunicationManager::GetInstance()->python_communication_object_->CallMethod(method, syntax, NULL);
            ret->ReleaseResources();
		}
		catch(...)
		{
			std::string text;
			text = "Cannot call method [";
			text.append(method_name);
			text.append("] with argument [NULL]");
			LogError(text);
		}
	}

	/**
	 * Wrapper for calling methods of python object.
	 *  \param method_name The method name
	 *  \param arg The string argument for the method
     *  \note The calls methods of python_communication_object_ with signature: (string method_name, "s", string arg)
	 */
	void CommunicationManager::CallPythonCommunicationObject(const std::string &method_name, const std::string &arg) const
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
            ret->ReleaseResources();
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
	}

	/**
	 *  Wrapper for calling methods of python objects.
	 *  \param method_name The method name
     *  \note The calls methods of python_communication_object_ with signature: (string method_name, "", NULL)
     *  returns python script object that the caller must release with ReleaseResources method
	 */
	Foundation::ScriptObject* CommunicationManager::CallPythonCommunicationObjectAndGetReturnValue(const std::string &method_name) const
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
            return NULL;
		}
    }

	/**
	 * Wrapper for calling methods of python object.
	 *  \param method_name The method name
	 *  \param arg The string argument for the method
     *  \note The calls methods of python_communication_object_ with signature: (string method_name, "s", string arg)
     *  returns python script object that the caller must release with ReleaseResources method
	 */
	Foundation::ScriptObject* CommunicationManager::CallPythonCommunicationObjectAndGetReturnValue(const std::string &method_name,  const std::string &arg) const
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
            return NULL;
		}        
    }

} // end of namespace: Communication




