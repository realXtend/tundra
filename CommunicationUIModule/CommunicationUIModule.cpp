#pragma warning( push )
#pragma warning( disable : 4250 )
#undef max
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/actiongroup.h>

#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

#include <Poco/ClassLibrary.h>

#include "StableHeaders.h"
#include "Foundation.h"



#include "ConfigureDlg.h"
#include "SelectionDialog.h"
#include "DialogCallbackInterface.h"

#include "ScriptServiceInterface.h"
#include "CommunicationUIModule.h"



namespace CommunicationUI
{

	CommunicationUIModule::CommunicationUIModule(void):ModuleInterfaceImpl("CommunicationUIModule"), online_status_list_ready_(false)
	{
	}

	CommunicationUIModule::~CommunicationUIModule(void)
	{
	}

	void CommunicationUIModule::Load()
	{
	}
	
	void CommunicationUIModule::Unload()
	{
	}

	void CommunicationUIModule::Initialize() 
	{
	}

	void CommunicationUIModule::PostInitialize()
	{
		initializeMainCommWindow();
        
		try
		{
			SetupCommunicationServiceUsage();
		}
		catch(std::string error)
		{
			std::string text = "PostInitialize failed: ";
			text.append(error);
			LogError(text);

			// \todo We should uninitialize comm ui here...
		}
	}

	void CommunicationUIModule::Uninitialize()
	{
        framework_->GetEventManager()->UnregisterEventSubscriber(this);
        SAFE_DELETE(wndCommMain);
	}

	void CommunicationUIModule::Update()
	{
	}

	void CommunicationUIModule::initializeMainCommWindow()
	{
		commUI_XML = Gnome::Glade::Xml::create("data/communicationUI.glade");
		if (!commUI_XML)
			return;
		wndCommMain = 0;
		                        
		commUI_XML->get_widget("wndCMain", wndCommMain);
        commUI_XML->get_widget("fxdContainer", fxdContainer);
        commUI_XML->get_widget("btnAddContact", btnAddContact);
        commUI_XML->get_widget("btnRemoveContact", btnRemoveContact);
        commUI_XML->get_widget("btnSetPresenceMessage", btnSetPresenceMessage);
        commUI_XML->get_widget("entryPresenceMessage", entryPresenceMessage);
        commUI_XML->get_widget("lblPresenceStatus", lblPresenceStatus);

        cmbPresence.set_size_request(240, 25);
        fxdContainer->put(cmbPresence, 25, 40);
        
        if(!wndCommMain||!fxdContainer)//||!wndSelection)
			return;

        lstContacts.set_size_request(240,182);
        fxdContainer->put(lstContacts, 25, 80);
        lstContacts.SetModule(*this);

	    commUI_XML->connect_clicked("mi_connect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));
		commUI_XML->connect_clicked("mi_disconnect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuDisconnect));
		commUI_XML->connect_clicked("mi_setaccount", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuSetAccountAndPassword));
		commUI_XML->connect_clicked("mi_settings", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuSettings));
        commUI_XML->connect_clicked("mi_createaccount", sigc::mem_fun(*this, &CommunicationUIModule::OnCreateAccount));

		commUI_XML->connect_clicked("mi_directchat", sigc::mem_fun(*this, &CommunicationUIModule::OnDirectChatMenuStartChat));
        
        commUI_XML->connect_clicked("btnAddContact", sigc::mem_fun(*this, &CommunicationUIModule::OnContactAdd));
        commUI_XML->connect_clicked("btnRemoveContact", sigc::mem_fun(*this, &CommunicationUIModule::OnContactRemove));
        //commUI_XML->connect_clicked("btnRefresh", sigc::mem_fun(*this, &CommunicationUIModule::OnRefresh));
        commUI_XML->connect_clicked("btnSetPresenceMessage", sigc::mem_fun(*this, &CommunicationUIModule::OnSetPresenceMessage));
        cmbPresence.signal_changed().connect(sigc::mem_fun(*this, &CommunicationUIModule::OnPresenceStatusSelection) );

		// entry dialog
		commUI_XML->connect_clicked("btnEntryOk", sigc::mem_fun(*this, &CommunicationUIModule::OnEntryDlgOk));
		commUI_XML->connect_clicked("btnEntryCancel", sigc::mem_fun(*this, &CommunicationUIModule::OnEntryDlgCancel));
        
        wndCommMain->set_title("Comms UI");
		wndCommMain->show();
        lstContacts.show();
        cmbPresence.show();
	}

	/**
	 *  - Fetches communication service object
	 *  - Subscribes communication events
	 *  - Gets contact list from communication service
	 */
	void CommunicationUIModule::SetupCommunicationServiceUsage()
	{
		communication_service_ = framework_->GetService<Communication::CommunicationServiceInterface>(Foundation::Service::ST_Communication).lock();
		if ( !communication_service_)
		{
			std::string error = "Cannot find communication service!";
			LogError(error);
			throw error;
		}

		framework_->GetEventManager()->RegisterEventSubscriber( framework_->GetModuleManager()->GetModule(this->Name()), 0, Foundation::ModuleWeakPtr() );
        communication_event_category_id_ = framework_->GetEventManager()->QueryEventCategory("Communication");
        
        if (communication_event_category_id_ == 0 )
		{
			std::string error = "Unable to find event category for Communication events!";
			LogError(error);
			throw error;
		}

		contact_list_ = communication_service_->GetContactList();
        comms_settings_ = communication_service_->GetCommunicationSettings();
	}

	void CommunicationUIModule::OnAccountMenuSettings()
	{
        // convert the settings used by communications to one used in communicationsui, since communications needs its own kind of settings?
        std::vector<std::string> keys = comms_settings_->GetProperties();
        std::map<std::string, SettingsAttribute> settingsMap;
        for(std::vector<std::string>::iterator iter = keys.begin();iter<keys.end(); iter++){
            SettingsAttribute attr;
			attr.type = CommunicationUI::String;
		    attr.value = comms_settings_->GetProperty(*(iter));
            settingsMap[*(iter)] = attr;
        }
        ConfigureDlg accDlg(settingsMap.size(), settingsMap, "connection settings", this);
		Gtk::Main::run(accDlg);
               
   //     for(std::vector<std::string>::iterator iter = settingsVect.begin(); iter<settingsVect.end()-1; iter+=2)
   //     {
   //         SettingsAttribute attr;
			//attr.type = CommunicationUI::String;
		 //   attr.value = *(iter+1);
   //         std::string key = *iter;
   //         attrs[key] = attr;
   //     }
  //      ConfigureDlg accDlg(settingsMap.size(), settingsMap, "connection settings", this);
		//Gtk::Main::run(accDlg);
	}

    void CommunicationUIModule::OnCreateAccount()
    {
        LogInfo("OnCreateAccount");
        std::string infoText = "Register account according to your connection settings";
        SelectionDialog sd("Register account", infoText , "_Ok", "_Cancel", this);
        Gtk::Main::run(sd);        
    }

	void CommunicationUIModule::OnAccountMenuSetAccountAndPassword()
	{
		std::map<std::string, SettingsAttribute> attributes; // attributes for ConfigureDlg object

		Communication::CommunicationSettingsInterfacePtr c = communication_service_->GetCommunicationSettings();
		std::vector<std::string> properties = c->GetProperties();
		for (std::vector<std::string>::iterator i = properties.begin(); i != properties.end(); i++)
		{
			std::string name = *i;
			SettingsAttribute attr;
			attr.type = CommunicationUI::String;
			attr.value = c->GetProperty(name);
			attributes[name] = attr;
		}

		ConfigureDlg accDlg(attributes.size(), attributes, "account settings", this);
		Gtk::Main::run(accDlg);
	}

	void CommunicationUIModule::OnAccountMenuConnect()
	{
        LogInfo("OnAccountMenuConnect");

		// \todo use user defined credential here!
		Communication::CommunicationSettingsInterfacePtr c = communication_service_->GetCommunicationSettings();
		communication_service_->OpenConnection(c);
	}
	void CommunicationUIModule::OnAccountMenuDisconnect()
	{
		communication_service_->CloseConnection();
	}

	/*
	 *  Show window where user can give IM address of partner to with start chat session
	 */
	void CommunicationUIModule::OnDirectChatMenuStartChat()
	{
		LogInfo("OnDirectChatMenuStartChat");
		Gtk::Entry* entryBox = (Gtk::Entry*)commUI_XML->get_widget("entryDialogInputTextArea");
		Gtk::Label* entryLabel = (Gtk::Label*)commUI_XML->get_widget("lblEntryDialogInfo");
		entryLabel->set_text("Give counter part address:");
		dlgEntry = (Gtk::Dialog*)commUI_XML->get_widget("dlgEntry");
		dlgEntry->set_title("Start direct chat");
		
		int resp = dlgEntry->run();

		Glib::ustring str = entryBox->get_text();
		LogInfo(str.c_str());
		if(entryret_==1){
            StartChat(str.c_str());
		}
        
	}

    void CommunicationUIModule::OnSetStatusOnline()
    {
		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("available");
		communication_service_->SetPresenceStatus( s );
    }

    void CommunicationUIModule::OnSetStatusAway()
    {
		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("away");
		communication_service_->SetPresenceStatus( s );
    }
    void CommunicationUIModule::OnSetStatusBusy()
    {
		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("busy");
		communication_service_->SetPresenceStatus( s );
    }

    void CommunicationUIModule::OnSetStatusOffline()
    {
		return; // Gabble connection manager will crash if we send this online status !!!

		// \bug
		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("offline");
		communication_service_->SetPresenceStatus( s );
    }

	/*
	 * \note This sould not be needed when using contact list directly.
	 *       But if we know just the address we need this to find actual contact
	 *       object.
	 * \todo Don't use protocol specific im address as a key for contacts
	 */
	Communication::ContactPtr CommunicationUIModule::FindContact(std::string address)
	{
		std::string protocol = "jabber"; // todo remove fixed definition
		for (Communication::ContactList::iterator i = contact_list_->begin(); i != contact_list_->end(); ++i)
		{
			Communication::ContactPtr c = (*i);
			if (c->GetContactInfo(protocol)->GetProperty("address").compare(address) == 0)
			{
				return c;
			}
		}

		// we didn't find given contact
		// \todo better error handling
		return Communication::ContactPtr();
	}

	// Direct chat command support 
	// \todo remove this
    void CommunicationUIModule::StartChat(const char* contact_address)
    {
		LogInfo("start chat window here");
		try
		{
			Communication::ContactPtr contact = FindContact(contact_address);	
			Communication::IMSessionPtr session = communication_service_->CreateIMSession(contact);
//			chatSessions_[std::string(contact_address)] = CommunicationUI::ChatSessionUIPtr(new CommunicationUI::ChatSession(session, communication_service_));
			chatSessions_.push_back( CommunicationUI::ChatSessionUIPtr(new CommunicationUI::ChatSession(session, communication_service_)) );
		}
		catch(...)
		{
			std::string text = "Cannot start IM session because cannot find contact: ";
			text.append(contact_address);
			LogError(text);
		}
    }

	void CommunicationUIModule::OnEntryDlgOk(){
		entryret_ = 1;
		dlgEntry->hide();
	}
	void CommunicationUIModule::OnEntryDlgCancel(){
		dlgEntry->hide();
		entryret_ = 0;
	}

    void CommunicationUIModule::OnContactAdd()
    {
        LogDebug("OnContactAdd");
		std::map<std::string, SettingsAttribute> attrs;
		SettingsAttribute contact_address;
		contact_address.type = CommunicationUI::String;
		contact_address.value = "";
		attrs["contact address"] = contact_address;
		int count = attrs.size();
        ConfigureDlg accDlg(count, attrs, "contact address", this);
		Gtk::Main::run(accDlg);
    }

    void CommunicationUIModule::OnContactRemove()
    {
        LogInfo("OnContactRemove");
        Gtk::TreeModel::iterator iter = lstContacts.GetSelected();
        if(iter)
        {
            std::string address = (*iter)[lstContacts.columns_.contact_];
			Communication::ContactPtr c = FindContact(address);
			if (c)
			{
				communication_service_->RemoveContact(c);
				return;
			}
        }
		std::string error = "Try to remove unknow contact";
		LogError(error);
    }

	// \todo remove this?
    void CommunicationUIModule::OnRefresh()
    {
        LogInfo("OnRefresh");
    }

    void CommunicationUIModule::OnSetPresenceMessage()
    {
        LogInfo("OnSetPresenceMessage");
        //Gtk::Entry* entrySetPresenceMessage;
        //Gtk::ComboBoxText cmbPresence;
        std::string status = cmbPresence.get_active_text();
        LogInfo("1");

        Glib::ustring pmessage = entryPresenceMessage->get_text();

        LogInfo("2");
        status.append(":");
        LogInfo("3");
        status.append(pmessage);
        LogInfo("call py");

		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineMessage(status);
		communication_service_->SetPresenceStatus( s );
    }

	void CommunicationUIModule::OnPresenceStatusSelection()
	{
        LogInfo("OnPresenceStatusSelection");
        std::string status = cmbPresence.get_active_text();
        LogInfo(status);

		if (!online_status_list_ready_)
			return; // list build process causes these events

		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus(status);
		communication_service_->SetPresenceStatus( s );
	}

	/**
	 *  \todo Implement load eg. with ConfigurationManager
	 */
	Communication::CommunicationSettingsInterfacePtr CommunicationUIModule::LoadCommunicationSettings()
	{
        Communication::CommunicationSettingsInterfacePtr csifp = communication_service_->GetCommunicationSettings();
        return csifp;
	}

	void CommunicationUIModule::Callback(std::string aConfigName, std::map<std::string, SettingsAttribute> attributes)
	{
        if(aConfigName=="account settings")
		{
			// TODO: Handle settings, rg. write to ConfigureManager
//            commManager->SetAccountAttributes(attributes);
        } else if(aConfigName=="connection settings")
		{
			LogInfo("Callback from settings dlg");
			// TODO: Replace this functionality with CredentialPtr usage
            for(std::map<std::string, SettingsAttribute>::const_iterator iter = attributes.begin(); iter!=attributes.end(); ++iter){
                std::string key = iter->first;
                CommunicationUI::SettingsAttribute attr = iter->second;
                std::string value = attr.value;
                comms_settings_->SetProperty(key, value);
            }
            comms_settings_->Save();

        } else if(aConfigName=="contact address") {
            SettingsAttribute sattr = attributes["contact address"];
			Communication::ContactInfo* info = new Communication::ContactInfo();
			info->SetProperty("address", sattr.value);
			communication_service_->SendFriendRequest(Communication::ContactInfoPtr(info));
        } else if(aConfigName=="Remote request")
		{
			// TODO: This must be checked
            //LogInfo("Remote request callback");
            //// send responce to other end
            //std::string answer = attributes.begin()->first;
            //SettingsAttribute user = attributes.begin()->second;
            //LogDebug(answer);
            //LogDebug(user.value);
            //if(answer=="Ok"){
            //    CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            //} else {
            //    CallIMPyMethod("CDenyContactRequest", "s", user.value);
            //}
        } else if(aConfigName=="Presence request"){
            LogInfo("Presence request callback");
            // send responce to other end
            std::string answer = attributes.begin()->first;
            SettingsAttribute user = attributes.begin()->second;
            LogDebug(answer);
            LogDebug(user.value);
			Communication::FriendRequestPtr r =	FindFriendRequest(user.value);
			if (!r)
			{
				std::string text = "Cannot find friend request: ";
				text.append(user.value);
				LogError(text);
				return; 
			}
			
            if(answer=="Ok"){
				r->Accept();
//                CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            } else {
				r->Deny();
//                CallIMPyMethod("CDenyContactRequest", "s", user.value);
            }
        } else if(aConfigName=="Contact accepted")
		{
			// TODO: Check this
            //LogInfo("Contact accepted callback");
            //// send responce to other end
            //std::string answer = attributes.begin()->first;
            //SettingsAttribute user = attributes.begin()->second;
            //LogDebug(answer);
            //LogDebug(user.value);
            //if(answer=="Ok"){
            //    CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            //} else {
            //    CallIMPyMethod("CDenyContactRequest", "s", user.value);
            //}
        } else if(aConfigName=="Subscribe incoming"){
            //LogInfo("Subscribe incoming callback");
            //std::string answer = attributes.begin()->first;
            //SettingsAttribute user = attributes.begin()->second;
            //LogInfo(answer);
            //if(answer=="Ok"){
            //    CallIMPyMethod("CSendSubscription", "s", user.value);
            //} else {
            //    CallIMPyMethod("CDenyContactRequest", "s", user.value);
            //}
        } else if(aConfigName=="Incoming contact request"){
            //LogInfo("Incoming contact request");
            //// send responce to other end
            //std::string answer = attributes.begin()->first;
            //SettingsAttribute user = attributes.begin()->second;
            //LogDebug(answer);
            //LogDebug(user.value);
            //if(answer=="Ok"){
            //    CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            //} else {
            //    CallIMPyMethod("CDenyContactRequest", "s", user.value);
            //}
        } else if(aConfigName=="Contact removed"){
            LogInfo("Contact removed");
        } else if(aConfigName=="Register account")
		{
			std::string answer = attributes.begin()->first;
            if(answer=="Ok"){				
                //Communication::CommunicationSettingsInterfacePtr c = LoadCommunicationSettings();
                this->communication_service_->CreateAccount();
				// TODO: Implement CreateAccount(c) in communication sevice interface
//				communication_service_->CreateAccount(c);
//            CallIMPyMethod("CCreateAccount", "", std::string(""));
            }
			else {
				// User canceled, we do nothing
            }

        } else {
            LogInfo("Unknown callback");
            LogInfo(aConfigName);
        }
		// TODO: Save eg. configure manager from where Communication Service can read them ?
//		if(aConfigName=="account settings"){ commManager->SetAccountAttributes(attributes); }
	}

    void CommunicationUIModule::OnContactListClicked()
    {
        LogInfo("OnContactListClicked");
    }
    
	/**
	 * @return Corresponding ChatSessionUIPtr object of given IMSessionPtr object
	 *         if it cannot be found then return null pointer
	 **/
	ChatSessionUIPtr CommunicationUIModule::GetUIChatSession(Communication::IMSessionPtr s)
	{
		for (ChatSessionVector::iterator i = chatSessions_.begin(); i != chatSessions_.end(); ++i)
		{
			ChatSessionUIPtr cs = *i;
			if ( cs->GetSession() == s )
				return cs;
		}

		std::string error = "We don't find session dlg instance for given session!";
		LogError(error);
		return ChatSessionUIPtr();
	}

	/**
	 * Creates a new ui session element for given im session
	 **/
	ChatSessionUIPtr CommunicationUIModule::CreateUIChatSession(Communication::IMSessionPtr s)
	{
		return ChatSessionUIPtr();
	}

	/**
 	 * Handles all communication service events
     **/
    bool CommunicationUIModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if ( category_id != communication_event_category_id_)
			return false;

		switch( event_id )
		{
		case Communication::Events::IM_MESSAGE:
			{
				Communication::Events::IMMessageEventInterface* e = (Communication::Events::IMMessageEventInterface*)(data);
				Communication::IMMessagePtr m = e->GetIMMessage();
				Communication::IMSessionPtr s = e->GetSession();

				ChatSessionUIPtr ui_session = GetUIChatSession(s);
				if (!ui_session)
				{
					// We have a new incoming session

					break;// todo: Handle situation
				}

				ui_session->OnMessageReceived(m);

				std::string message_text = m->GetText();
				std::string text;
				text = "Got event IM_MESSAGE: ";
				text.append(message_text);
				LogInfo(text);
			}
			return true;
			break;

		case Communication::Events::IM_SESSION_REQUEST:
			{
				Communication::Events::IMSessionRequestEventInterface* e = (Communication::Events::IMSessionRequestEventInterface*)(data);
				Communication::IMSessionPtr s = e->GetSession();
				Communication::ContactPtr c = e->GetContact();
				std::string protocol = s->GetProtocol();
				std::string address = c->GetContactInfo(protocol)->GetProperty("address");
				std::string text;
				text = "Got event IM_SESSION_REQUEST: ";
				text.append(address);
				LogInfo(text);

				HandleIncomingIMSession(s);
			}
			return true;
			break;

		case Communication::Events::PRESENCE_STATUS_UPDATE:
			{
				Communication::Events::PresenceStatusUpdateEventInterface* e = (Communication::Events::PresenceStatusUpdateEventInterface*)(data);
				Communication::ContactPtr c = e->GetContact();
				std::string status = c->GetPresenceStatus()->GetOnlineStatus();
				std::string text;
				text = "Got event PRESENCE_STATUS_UPDATE: ";
				text.append(status);
				LogInfo(text);
				UpdateContactList();
			}
			return true;
			break;

		case Communication::Events::FRIEND_REQUEST:
			{
				Communication::Events::FriendRequestEventInterface* e = (Communication::Events::FriendRequestEventInterface*)(data);
				Communication::FriendRequestPtr r = e->GetFriendRequest();
				Communication::ContactInfoPtr info = r->GetContactInfo();
				std::string address = info->GetProperty("address");
				std::string text;
				text = "Got event FRIEND_REQUEST: ";
				text.append(address);
				LogInfo(text);
				HandleFriendRequest(r);
			}
			return true;
			break;

		case Communication::Events::FRIEND_RESPONSE:
			LogInfo("Got event: FRIEND_RESPONSE");
			return true;
			break;

		case Communication::Events::CONNECTION_STATE:
			{
				Communication::Events::ConnectionStateEventInterface* e = (Communication::Events::ConnectionStateEventInterface*)(data);
				std::string type_text;
                Gtk::Label* label = (Gtk::Label*)commUI_XML->get_widget("lblOnlineStatus");
				switch(e->GetType())
				{
				case Communication::Events::ConnectionStateEventInterface::CONNECTION_OPEN:
					type_text = "CONNECTION_OPEN";
		            label->set_label("Connected");
					break;

				case Communication::Events::ConnectionStateEventInterface::CONNECTION_CLOSE:
					type_text = "CONNECTION_CLOSE";
		            label->set_label("Disconnected");
					break;

				case Communication::Events::ConnectionStateEventInterface::CONNECTION_STATE_UPDATE:
					UpdateOnlineStatusList();
					UpdateContactList();
					type_text = "CONNECTION_STATE_UPDATE";
					break;

                case Communication::Events::ConnectionStateEventInterface::CONNECTION_CONNECTING:
                    type_text = "CONNECTING";
		            label->set_label("Connecting");
                    break;
				}
				std::string text;
				text = "Got event SESSION_STATE: ";
				text.append(type_text);
				LogInfo(text);
			}
			return true;
			break;

		case Communication::Events::SESSION_STATE:
			{
				Communication::Events::SessionStateEventInterface* e = (Communication::Events::SessionStateEventInterface*)(data);
				Communication::IMSessionPtr s =  e->GetIMSession();

				int event_type = e->GetType();
				switch(event_type)
				{
				case Communication::Events::SessionStateEventInterface::SESSION_END:
					{
						ChatSessionUIPtr chat_dlg = GetUIChatSession(s);
						if (chat_dlg)
							chat_dlg->OnStateChanged();
					}
					break;
				}
			}
        case Communication::Events::ACCOUNT_CREATION:
			{
				Communication::Events::SessionStateEventInterface* e = (Communication::Events::SessionStateEventInterface*)(data);
				int event_type = e->GetType();
				switch(event_type)
				{
                case Communication::Events::AccountCreationEventInterface::ACCOUNT_CREATION_SUCCEEDED:
                    {
                        std::string infoText("Account created!");
                        SelectionDialog sd("Account creation succeeded", infoText , "_Ok", "_Cancel", this, false);
                        Gtk::Main::run(sd);        
                        
                    }
                    break;
                case Communication::Events::AccountCreationEventInterface::ACCOUNT_CREATION_FAILED:
                    //\bug the reason why account is not created, is currently not passed to user
                    {
                        std::string infoText("Account creation failed!");
                        SelectionDialog sd("Account creation failed", infoText , "_Ok", "_Cancel", this, false);
                        Gtk::Main::run(sd);        
                        
                    }
                    break;
                }
            }

		}
       
        return false;
    }    

	void CommunicationUIModule::ShowFriendRequestWindow(Communication::FriendRequestPtr r)
	{
		std::string id = r->GetContactInfo()->GetProperty("address");
		friend_requests_[id] = r;

        std::string infoText = "Let user ";
        infoText.append(id);
        infoText.append(" see your presence status");
        SelectionDialog sd("Presence request", infoText, "_Allow", "_Deny", this, std::string(id));
        Gtk::Main::run(sd);
	}
	
	/**
	 * 
	 */
	Communication::FriendRequestPtr	CommunicationUIModule::FindFriendRequest(std::string address)
	{
		FriendRequestMap::iterator i = friend_requests_.find(address);
		if (i == friend_requests_.end())
			return Communication::FriendRequestPtr(); // didn't find the request!!
		return i->second;
	}

	void CommunicationUIModule::HandleFriendRequest(Communication::FriendRequestPtr r)
	{
		ShowFriendRequestWindow(r);
	}
	

	/*
	 *  Get all online status options from communication service and fills
	 *  drop down menu with these options and selects current state.
	 */
	void CommunicationUIModule::UpdateOnlineStatusList()
	{
		online_status_list_ready_ = false;
		cmbPresence.clear();
		std::vector<std::string> options = communication_service_->GetPresenceStatus()->GetOnlineStatusOptions();
        for(std::vector<std::string>::iterator iter = options.begin(); iter != options.end(); ++iter)
		{
			std::string option = (*iter);
            cmbPresence.append_text(option);
        }
		cmbPresence.set_active_text(communication_service_->GetPresenceStatus()->GetOnlineStatus());
		online_status_list_ready_ = true;
	}

	/**
	 *  Refresh contact list ui element
	 */
	void CommunicationUIModule::UpdateContactList()
	{
		contact_list_ = communication_service_->GetContactList();
		this->lstContacts.lstContactsTreeModel->clear();
		for(Communication::ContactList::iterator i = contact_list_->begin(); i < contact_list_->end(); i++)
		{
			Communication::ContactPtr c = (*i);

			Gtk::TreeModel::Row row = *(this->lstContacts.lstContactsTreeModel->append());
			row[lstContacts.columns_.id_] = "";  // \todo We don't need first column...
			row[lstContacts.columns_.contact_] = c->GetName();
			row[lstContacts.columns_.status_] = c->GetPresenceStatus()->GetOnlineStatus();
			row[lstContacts.columns_.message_] = c->GetPresenceStatus()->GetOnlineMessage();
		}
	}

	/**
	 *  Creates ui session object (a dialog) and popups a new window for given session object
	 **/
	void CommunicationUIModule::HandleIncomingIMSession(Communication::IMSessionPtr s)
	{
		ChatSessionUIPtr chat_session = GetUIChatSession(s);
		if (!chat_session)
		{
			chat_session = ChatSessionUIPtr( new ChatSession(s, communication_service_) );
			chatSessions_.push_back(chat_session);
		}
		chat_session->ChannelOpen();
	}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace CommunicationUI;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationUIModule)
POCO_END_MANIFEST
