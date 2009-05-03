
#include "StableHeaders.h"
#include "Foundation.h"


//#include "EventDataInterface.h"
#include "TelepathyCommunication.h"

namespace Communication
{
	using namespace Communication;

	TelepathyCommunicationPtr TelepathyCommunication::instance_;
	void (TelepathyCommunication::*testCallbackPtr)(char*) = NULL;  

	TelepathyCommunication::TelepathyCommunication(Foundation::Framework *f)
	{
		TelepathyCommunication::instance_ = TelepathyCommunicationPtr(this);
		framework_ = f;

		InitializePythonCommunication();
		RegisterEvents();
	}

	TelepathyCommunication::~TelepathyCommunication()
	{
		// todo: Cleanup
	}

	// rename to: InitPythonCommunication
	void TelepathyCommunication::InitializePythonCommunication()
	{
		Foundation::ScriptServiceInterface *scriptService = framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting);
		std::string error;
		this->communication_py_script_ = Foundation::ScriptObjectPtr(scriptService->LoadScript(COMMUNICATION_SCRIPT_NAME, error));
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
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackTest, "key");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackConnected, "connected");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackConnecting, "disconnected");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackDisconnected, "connecting");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackChannelOpened, "channel_opened");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackChannelClosed, "channel_closed");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackMessagReceived, "message_received");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PycallbackFriendReceived, "contact_item");
		((Foundation::ScriptEventInterface*)scriptService)->SetCallback( &TelepathyCommunication::PyCallbackContactStatusChanged, "contact_status_changed");
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
	  Uses credential from Account.txt
	*/
	void TelepathyCommunication::OpenConnection(CredentialsPtr c)
	{
			std::string method = "CAccountConnect";
			std::string syntax = "";
			Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
	}

	void TelepathyCommunication::CloseConnection()
	{
		std::string method = "CDisconnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, NULL);
	}

	IMSessionPtr TelepathyCommunication::CreateIMSession(ContactPtr contact)
	{
		bool jabber_contact_founded = false;
		ContactInfoPtr jabber_contact;
		ContactInfoList list = contact->GetContactInfos();
		for (int i=0; i<list.size(); i++)
		{
			if ( ((ContactInfoPtr)(list[i]))->GetType().compare("jabber")==0 )
			{
				jabber_contact = (ContactInfoPtr)(list[i]);
				jabber_contact_founded = true;
				break;
			}
		}

		if (!jabber_contact_founded)
		{
			// TODO: Handle error situation
		}

		int session_id = 0; // todo replace by real id
		TPIMSession* session = new TPIMSession(python_communication_object_);
		TPIMSessionPtr session_ptr = TPIMSessionPtr(session);
		this->im_sessions_.push_back(session_ptr);

		char** args = new char*[1];
		char* buf1 = new char[100];
		strcpy(buf1,jabber_contact->GetValue().c_str());
		args[0] = buf1;

		std::string method = "CStartChatSession"; // todo: CCreateIMSessionJabber, CCreateIMSessionSIP, etc.
		std::string syntax = "s";
		Foundation::ScriptObject* ret = python_communication_object_->CallMethod(method, syntax, args);
		return IMSessionPtr( (IMSession*)session);
		// TODO: Get session id from python !

		return IMSessionPtr( (IMSession*)session);
	}

	ContactList TelepathyCommunication::GetContactList()
	{
		return contact_list_;
	}

	TelepathyCommunicationPtr TelepathyCommunication::GetInstance()
	{
		return TelepathyCommunication::instance_;
	}

	void TelepathyCommunication::PublishPresence(PresenceStatusPtr p)
	{
		// TODO: call python here
	}


	// PYTHON CALLBACK HANDLERS: 



	/*
	   Called by communication.py via PythonScriptModule
	   For internal test from python 
	*/
	void TelepathyCommunication::PyCallbackTest(char* t)
	{
		// do tests here
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When request connection is open
	*/
	void TelepathyCommunication::PyCallbackConnected(char*)
	{
		// todo : Set own online status to: Online
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When requested connection is "connecting" state (witch phase of that state???)
	*/
	void TelepathyCommunication::PyCallbackConnecting(char*)
	{
		// todo : Do we need this? Send notify for UI to be "connecting state"?
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When connection is closed
	*/
	void TelepathyCommunication::PyCallbackDisconnected(char*)
	{
		// todo : Set own online status to: Offline
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When new session was opened
	   todo: session id should be received
	   todo: IS THIS INCOMING OR OUTGOING ???
	*/
	void TelepathyCommunication::PyCallbackChannelOpened(char*)
	{
		// todo: create session
		TPIMSessionPtr s = TPIMSessionPtr( new TPIMSession(TelepathyCommunication::GetInstance()->python_communication_object_) );
		IMSessionInvitationEvent e = IMSessionInvitationEvent( IMSessionPtr((IMSession*) s.get() ) );
		TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_SESSION_INVITATION_RECEIVED, (Foundation::EventDataInterface*)&e);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When (the only) session is closed by remote end
	   // TODO: Session id sould be received
	*/
	void TelepathyCommunication::PyCallbackChannelClosed(char*)
	{
		int session_id = 0;
		for (int i = 0; i < TelepathyCommunication::GetInstance()->im_sessions_.size(); i++)
		{
			TPIMSessionPtr s = (TPIMSessionPtr)TelepathyCommunication::GetInstance()->im_sessions_[i];
			if (s->GetId() == session_id)
			{
				s->NotifyClosedByRemote();
				IMSessionClosedEvent e = IMSessionClosedEvent((IMSessionPtr( (IMSession*)s.get() )));
				TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::SESSION_CLOSED, (Foundation::EventDataInterface*)&e);
				// todo: remove session from im_sessions_
			}
		}
		
		//IMMessageReceivedEvent e = IMMessageReceivedEvent(IMMessagePtr( (IMMessage*)m.get() ));
		//TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE_RECEIVED, (Foundation::EventDataInterface*)&e);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When message is received
	   todo: session id !!!
	*/
	void TelepathyCommunication::PyCallbackMessagReceived(char* text)
	{
		int session_id = 0; // todo: Get real session id from python
		TPIMMessagePtr m = TPIMMessagePtr(new TPIMMessage(session_id));
		m->SetText(text);

		for (int i = 0; i < TelepathyCommunication::GetInstance()->im_sessions_.size(); i++)
		{
			TPIMSessionPtr s = (TPIMSessionPtr)TelepathyCommunication::GetInstance()->im_sessions_[i];
			if (s->GetId() == session_id)
			{
				s->NotifyMessageReceived(m);
			}
		}
		
		IMMessageReceivedEvent e = IMMessageReceivedEvent(IMMessagePtr( (IMMessage*)m.get() ));
		TelepathyCommunication::GetInstance()->event_manager_->SendEvent(TelepathyCommunication::GetInstance()->comm_event_category_, Communication::Events::IM_MESSAGE_RECEIVED, (Foundation::EventDataInterface*)&e);
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When ???
	*/
	void TelepathyCommunication::PycallbackFriendReceived(char* t)
	{
		// todo: handle this
	}

	/*
	   Called by communication.py via PythonScriptModule
	   When presence update is received
	   // todo: What data is received ???
	*/
	void TelepathyCommunication::PyCallbackContactStatusChanged(char* id)
	{
		// todo: Update presence object
		//       Create event
		//       send event

//		PresenceStatusUpdateEvent e = PresenceStatusUpdateEvent();
	//	UpdatePresence
		
	}

} // end of namespace: Communication
