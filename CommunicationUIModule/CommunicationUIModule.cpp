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
	CommunicationUIModule* CommunicationUIModule::instance_;

	CommunicationUIModule::CommunicationUIModule(void):ModuleInterfaceImpl("CommunicationUIModule")
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
		//commManager = (framework_->GetService<Foundation::Comms::CommunicationManagerServiceInterface>(Foundation::Service::ST_CommunicationManager)).lock();
        //commManager = framework_->GetService<Foundation::Comms::CommunicationManagerServiceInterface>(Foundation::Service::ST_CommunicationManager);
//                      (framework_->GetService<Scene::SceneManager>(Foundation::Service::ST_SceneManager)).lock();
		//commManager = framework_->GetService<Foundation::Comms::CommunicationManagerServiceInterface>(Foundation::Service::ST_CommunicationManager).lock();

        commManager = framework_->GetService<Foundation::Comms::CommunicationManagerServiceInterface>(Foundation::Service::ST_CommunicationManager).lock();

		CommunicationUIModule::instance_= this;		
        
        setupSciptInterface();		
		SetupCommunicationServiceUsage();
	}

	void CommunicationUIModule::Uninitialize()
	{
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
        
        commUI_XML->connect_clicked("mi_online", sigc::mem_fun(*this, &CommunicationUIModule::OnSetStatusOnline));
        commUI_XML->connect_clicked("mi_away", sigc::mem_fun(*this, &CommunicationUIModule::OnSetStatusAway));
        commUI_XML->connect_clicked("mi_busy", sigc::mem_fun(*this, &CommunicationUIModule::OnSetStatusBusy));
        commUI_XML->connect_clicked("mi_offline", sigc::mem_fun(*this, &CommunicationUIModule::OnSetStatusOffline));

		//commUI_XML->connect_clicked("btnTest", sigc::mem_fun(*this, &CommunicationUIModule::reloadIMScript));
        commUI_XML->connect_clicked("btnTest", sigc::mem_fun(*this, &CommunicationUIModule::testDialog));
        commUI_XML->connect_clicked("btnAddContact", sigc::mem_fun(*this, &CommunicationUIModule::OnContactAdd));
        commUI_XML->connect_clicked("btnRemoveContact", sigc::mem_fun(*this, &CommunicationUIModule::OnContactRemove));
        commUI_XML->connect_clicked("btnRefresh", sigc::mem_fun(*this, &CommunicationUIModule::OnRefresh));
        commUI_XML->connect_clicked("btnSetPresenceMessage", sigc::mem_fun(*this, &CommunicationUIModule::OnSetPresenceMessage));



        //commUI_XML->connect_property_changed("cmbPresence", sigc::mem_fun(*this, &CommunicationUIModule::OnComboChange));
        cmbPresence.signal_changed().connect(sigc::mem_fun(*this, &CommunicationUIModule::OnComboChange) );


		// entry dialog
		commUI_XML->connect_clicked("btnEntryOk", sigc::mem_fun(*this, &CommunicationUIModule::OnEntryDlgOk));
		commUI_XML->connect_clicked("btnEntryCancel", sigc::mem_fun(*this, &CommunicationUIModule::OnEntryDlgCancel));
        
        //wndCommMain->set_name("Comms UI");
        wndCommMain->set_title("Comms UI");
		wndCommMain->show();
        lstContacts.show();
        cmbPresence.show();
	}

    void CommunicationUIModule::testDialog()
    {
        //SelectionDialog sd("test", "Go home", "_Yep", "_No Way!", this);
        //Gtk::Main::run(sd);
		std::string str = "CTest";
		std::string syntax = "";
        Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, NULL);
    }

    void CommunicationUIModule::reloadIMScript()
    {
        delete this->imScriptObject;
        this->imScriptObject = NULL;
        setupSciptInterface();
    }

	
		
	/*
	 *  - Fetches communication service object
	 *  - Subscribes communication events
	 *  - Get contact list 
	 */
	void CommunicationUIModule::SetupCommunicationServiceUsage()
	{
		communication_service_ = framework_->GetService<Communication::CommunicationServiceInterface>(Foundation::Service::ST_Communication).lock();
		if (communication_service_.get() == NULL)
		{
			LogError("Cannot find communication service!");
			return;
		}

		framework_->GetEventManager()->RegisterEventSubscriber( framework_->GetModuleManager()->GetModule(this->Name()), 0, Foundation::ModuleWeakPtr() );
        communication_event_category_id_ = framework_->GetEventManager()->QueryEventCategory("Communication");
        
        if (communication_event_category_id_ == 0 )
            LogWarning("Unable to find event category for Communication events!");

		contact_list_ = communication_service_->GetContactList();

	}

    void CommunicationUIModule::setupSciptInterface()
    {
        //scriptService = (framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting)).lock();
        //scriptService = framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting);
		//scriptService = framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting).lock();
        scriptService = framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting).lock();
        BOOST_ASSERT(scriptService!=NULL);

		std::string error;

        

		Foundation::ScriptObject* script = scriptService->LoadScript("IMDemo", error);

		if(error=="None")
		{
			this->imScriptObject = script->GetObject("IMDemo");
			std::string str = "CDoStartUp";
			std::string syntax = "";
			Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, NULL);
            //Foundation::ScriptObject* ret = CallIMPyMethod("CDoStartUp", "s", NULL);
		}
        //boost::shared_ptr<Foundation::ScriptEventInterface> eIntf 
        //    = dynamic_cast<boost::shared_ptr<Foundation::ScriptEventInterface>>(this->scriptService);
        //boost::shared_ptr<Foundation::ScriptEventInterface> eIntf = (boost::shared_ptr<Foundation::ScriptEventInterface)(this->scriptService);
		//Foundation::ScriptEventInterface *eIntf = dynamic_cast<Foundation::ScriptEventInterface*>(this->scriptService);
		Foundation::ScriptEventInterface *eIntf = dynamic_cast<Foundation::ScriptEventInterface*>(this->scriptService.get());

        //Foundation::ScriptEventInterface* eIntf = dynamic_cast<Foundation::ScriptEventInterface*>(scriptService.get());

		sessionUp_ = false;

		eIntf->SetCallback(CommunicationUIModule::testCallback, "key");
		eIntf->SetCallback(CommunicationUIModule::connected, "connected");
		eIntf->SetCallback(CommunicationUIModule::disconnected, "disconnected");
		eIntf->SetCallback(CommunicationUIModule::disconnected, "connecting");

		eIntf->SetCallback(CommunicationUIModule::channelOpened, "channel_opened");
		eIntf->SetCallback(CommunicationUIModule::channelClosed, "channel_closed");
		eIntf->SetCallback(CommunicationUIModule::messagReceived, "message_received");
		eIntf->SetCallback(CommunicationUIModule::contactReceived, "contact_item");

        eIntf->SetCallback(CommunicationUIModule::contactAdded, "contact_added");
        eIntf->SetCallback(CommunicationUIModule::contactAddedToPublishList, "contact_added_publish_list");
        eIntf->SetCallback(CommunicationUIModule::contactRemoved, "contact_removed");
        eIntf->SetCallback(CommunicationUIModule::remotePending, "remote_pending");
        eIntf->SetCallback(CommunicationUIModule::localPending, "local_pending");

        eIntf->SetCallback(CommunicationUIModule::incomingRequest, "incoming_request");

        eIntf->SetCallback(CommunicationUIModule::contactStatusChanged, "contact_status_changed");
        eIntf->SetCallback(CommunicationUIModule::handleAvailableStatusList, "got_available_status_list");

        eIntf->SetCallback(CommunicationUIModule::handleRegisteringSuccess, "account_registering_succeeded");
        eIntf->SetCallback(CommunicationUIModule::handleRegisteringFailure, "account_registering_failed");
    }

	void CommunicationUIModule::OnAccountMenuSettings()
	{
        LogInfo("OnAccountMenuSettings");
        Foundation::ScriptObject* so = this->CallIMPyMethod("CGetSettings", "", std::string(""));
        char* retVal = so->ConvertToChar();
        std::string settingsStr(retVal);
        LogInfo(settingsStr);
        std::vector<std::string> settingsVect = this->SplitString(settingsStr, ":", 0);
        // code the vector into a map that config dialog understands
        std::map<std::string, Foundation::Comms::SettingsAttribute> attrs;
		
        for(std::vector<std::string>::iterator iter = settingsVect.begin(); iter<settingsVect.end()-1; iter+=2)
        {
            Foundation::Comms::SettingsAttribute attr;
		    attr.type = Foundation::Comms::String;
		    attr.value = *(iter+1);
            std::string key = *iter;
            attrs[key] = attr;
        }

        ConfigureDlg accDlg(attrs.size(), attrs, "connection settings", this);
        Gtk::Main::run(accDlg);
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
		//LogInfo("Set account");
		std::map<std::string, Foundation::Comms::SettingsAttribute> attributes = commManager->GetAccountAttributes();
		int count = attributes.size();
		LogInfo(attributes.find("name")->first);

		ConfigureDlg accDlg(count, attributes, "account settings", this);
		Gtk::Main::run(accDlg);
	}

	void CommunicationUIModule::OnAccountMenuConnect()
	{
		// TODO: use user defines credential 
        LogInfo("OnAccountMenuConnect");
        //
		Communication::Credentials* c = new Communication::Credentials();
		communication_service_->OpenConnection(Communication::CredentialsPtr(c));
        /*/
		std::string str = "CAccountConnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, NULL);
        //*/
	}
	void CommunicationUIModule::OnAccountMenuDisconnect()
	{
		//std::string str = "CDisconnect";
		//std::string syntax = "";
		//Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, NULL);
		communication_service_->CloseConnection();
	}

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

	// todo: Use user defines values
    void CommunicationUIModule::OnSetStatusOnline()
    {
		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("available");
		communication_service_->SetPresenceStatus( s );
        //std::cout << "OnSetStatusOnline" << std::endl;
        //this->CallIMPyMethod("CSetStatus", "s", std::string("available"));
    }
    void CommunicationUIModule::OnSetStatusAway()
    {
		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("away");
		communication_service_->SetPresenceStatus( s );
//        this->CallIMPyMethod("CSetStatus", "s", std::string("away"));
    }
    void CommunicationUIModule::OnSetStatusBusy()
    {
		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("busy");
		communication_service_->SetPresenceStatus( s );
        //std::cout << "OnSetStatusBusy" << std::endl;
        //this->CallIMPyMethod("CSetStatus", "s", std::string("busy"));
    }
    void CommunicationUIModule::OnSetStatusOffline()
    {
		return; // Gabble connection manager will crash if we send this online status !!!

		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus("offline");
		communication_service_->SetPresenceStatus( s );
        //std::cout << "OnSetStatusOffline" << std::endl;
        //this->CallIMPyMethod("CSetStatus", "s", std::string("offline"));
    }

	/*
	 * NOTE: This sould not be needed when using contact list directly
	 *       But if we know just the address we need this to find actual contat
	 *       object
	 */
	Communication::ContactPtr CommunicationUIModule::FindContact(std::string address)
	{
		std::string protocol = "jabber"; // todo remove fixed definition
		for (Communication::ContactList::iterator i = contact_list_->begin(); i < contact_list_->end(); i++)
		{
			Communication::ContactPtr c = (*i);
			if (c->GetContactInfo(protocol)->GetProperty("address").compare(address) == 0)
			{
				return c;
			}
		}

		// we didn't find given contact
		// todo: better error handling
		throw "Cannot find contact";
	}

    void CommunicationUIModule::StartChat(const char* contact_address)
    {
		LogInfo("start chat window here");
//        Foundation::ScriptObject* ret = CallIMPyMethod("CStartChatSession", "s", std::string(contact));
        //sessionUp_ = true;
		//this->session_ = CommunicationUI::ChatSessionUIPtr(new CommunicationUI::ChatSession(contact, imScriptObject));
		try
		{
			Communication::ContactPtr contact = FindContact(contact_address);	
			Communication::IMSessionPtr session = communication_service_->CreateIMSession(contact);
			chatSessions_[std::string(contact_address)] = CommunicationUI::ChatSessionUIPtr(new CommunicationUI::ChatSession(session, communication_service_));
//			chatSessions_[std::string(contact)] = CommunicationUI::ChatSessionUIPtr(new CommunicationUI::ChatSession(contact, imScriptObject));
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
		std::map<std::string, Foundation::Comms::SettingsAttribute> attrs;
		Foundation::Comms::SettingsAttribute contact_address;
		contact_address.type = Foundation::Comms::String;
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
            std::string id = (*iter)[lstContacts.columns_.id_];
            std::string contact = (*iter)[lstContacts.columns_.contact_];
            Foundation::ScriptObject* ret = CallIMPyMethod("CRemoveContact", "s", contact);
        }
    }

    void CommunicationUIModule::OnRefresh()
    {
        LogInfo("OnRefresh");
        Foundation::ScriptObject* ret = CallIMPyMethod("CRefreshContactStatusList", "", std::string(""));
    }

    void CommunicationUIModule::OnSetPresenceMessage()
    {
        LogInfo("OnSetPresenceMessage");
        //Gtk::Entry* entrySetPresenceMessage;
        //Gtk::ComboBoxText cmbPresence;
        std::string status = cmbPresence.get_active_text();
        LogInfo("1");

        Glib::ustring pmessage = entryPresenceMessage->get_text();
        //std::string pmessage = entrySetPresenceMessage->get_text();

        LogInfo("2");
        status.append(":");
        LogInfo("3");
        status.append(pmessage);
        LogInfo("call py");

		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineMessage(status);
		communication_service_->SetPresenceStatus( s );
//        this->CallIMPyMethod("CSetStatus", "s", status);
    }

    void CommunicationUIModule::OnComboChange()
    {
        LogInfo("OnComboChange");
        std::string status = cmbPresence.get_active_text();
        LogInfo(status);

		Communication::PresenceStatusPtr s = communication_service_->GetPresenceStatus();
		s->SetOnlineStatus(status);
		communication_service_->SetPresenceStatus( s );
//        this->CallIMPyMethod("CSetStatus", "s", status);
    }

	void CommunicationUIModule::Callback(std::string aConfigName, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes)
	{
        if(aConfigName=="account settings"){
            commManager->SetAccountAttributes(attributes);
        } else if(aConfigName=="connection settings") {
            LogInfo("Callback from settings dlg");
            //commManager->SetAccountAttributes(attributes);
            // code to attributes into one string and call save
            std::string saveString;
            std::map<std::string, Foundation::Comms::SettingsAttribute>::const_iterator iter;	
            //for(iter = attributes.begin(); iter!=attributes.end(); iter++){
            for(iter = attributes.begin(); iter!=attributes.end(); ++iter){
                std::string key = iter->first;
                Foundation::Comms::SettingsAttribute attr = iter->second;
                saveString.append(key + ":" + attr.value + ":");                
            }
            // remove the last ":"
            saveString = saveString.substr(0, saveString.size()-1);
            LogInfo(saveString);
            this->CallIMPyMethod("CSaveSettings", "s", saveString);
        } else if(aConfigName=="contact address") {
            Foundation::Comms::SettingsAttribute sattr = attributes["contact address"];
            CallIMPyMethod("CAddContact", "s", sattr.value);
        } else if(aConfigName=="Remote request"){
            LogInfo("Remote request callback");
            // send responce to other end
            std::string answer = attributes.begin()->first;
            Foundation::Comms::SettingsAttribute user = attributes.begin()->second;
            LogDebug(answer);
            LogDebug(user.value);
            if(answer=="Ok"){
                CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            } else {
                CallIMPyMethod("CDenyContactRequest", "s", user.value);
            }
        } else if(aConfigName=="Presence request"){
            LogInfo("Presence request callback");
            // send responce to other end
            std::string answer = attributes.begin()->first;
            Foundation::Comms::SettingsAttribute user = attributes.begin()->second;
            LogDebug(answer);
            LogDebug(user.value);
            if(answer=="Ok"){
                CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            } else {
                CallIMPyMethod("CDenyContactRequest", "s", user.value);
            }
        } else if(aConfigName=="Contact accepted"){
            LogInfo("Contact accepted callback");
            // send responce to other end
            std::string answer = attributes.begin()->first;
            Foundation::Comms::SettingsAttribute user = attributes.begin()->second;
            LogDebug(answer);
            LogDebug(user.value);
            if(answer=="Ok"){
                CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            } else {
                CallIMPyMethod("CDenyContactRequest", "s", user.value);
            }
        } else if(aConfigName=="Subscribe incoming"){
            LogInfo("Subscribe incoming callback");
            std::string answer = attributes.begin()->first;
            Foundation::Comms::SettingsAttribute user = attributes.begin()->second;
            LogInfo(answer);
            if(answer=="Ok"){
                CallIMPyMethod("CSendSubscription", "s", user.value);
            } else {
                CallIMPyMethod("CDenyContactRequest", "s", user.value);
            }
        } else if(aConfigName=="Incoming contact request"){
            LogInfo("Incoming contact request");
            // send responce to other end
            std::string answer = attributes.begin()->first;
            Foundation::Comms::SettingsAttribute user = attributes.begin()->second;
            LogDebug(answer);
            LogDebug(user.value);
            if(answer=="Ok"){
                CallIMPyMethod("CAcceptContactRequest", "s", user.value);
            } else {
                CallIMPyMethod("CDenyContactRequest", "s", user.value);
            }
        } else if(aConfigName=="Contact removed"){
            LogInfo("Contact removed");
        } else if(aConfigName=="Register account"){
            CallIMPyMethod("CCreateAccount", "", std::string(""));
        } else {
            LogInfo("Unknown callback");
            LogInfo(aConfigName);
        }
		if(aConfigName=="account settings"){ commManager->SetAccountAttributes(attributes); }
	}



	void CommunicationUIModule::setOnlineStatus(char* status)
	{

		Gtk::Label* label = (Gtk::Label*)instance_->commUI_XML->get_widget("lblOnlineStatus");
		label->set_label(status);
	}

	void CommunicationUIModule::testCallback(char* t)
	{
	}

	void CommunicationUIModule::connected(char* t)
	{
		LogInfo("connected");
		instance_->setOnlineStatus("connected");
	}

	void CommunicationUIModule::connecting(char*)
	{
		LogInfo("connecting");
		instance_->setOnlineStatus("connecting");
	}

	void CommunicationUIModule::disconnected(char* t)
	{
		LogInfo("disconnected");
		instance_->setOnlineStatus("disconnected");
        instance_->clearContactList();
	}

	void CommunicationUIModule::channelOpened(char* addr)
	{
		LogInfo("channelOpened");
		//if(instance_->session_!=NULL&&instance_->sessionUp_!=false){
		//	instance_->session_->ChannelOpen();
		//} else {
		//	instance_->session_ = CommunicationUI::ChatSessionUIPtr(new CommunicationUI::ChatSession(addr, instance_->imScriptObject));
		//}
		//instance_->sessionUp_ = true;
        std::map<std::string, ChatSessionUIPtr>::iterator iter = instance_->chatSessions_.find(std::string(addr));
        if(iter == instance_->chatSessions_.end())
		{
//			Communication::IMSessionPtr session = communication_service_->CreateIMSession(
            instance_->chatSessions_[std::string(addr)] = ChatSessionUIPtr(new ChatSession(addr, instance_->imScriptObject));
        } 
        instance_->chatSessions_[std::string(addr)]->ChannelOpen();
	}

	void CommunicationUIModule::channelClosed(char* t)
	{
		LogInfo("channelClosed");
		if(instance_->session_!=NULL){
			instance_->session_->ChannelClosed();
		} 
		instance_->sessionUp_ = false;
	}

	void CommunicationUIModule::messagReceived(char* addr_mess)
	{
		if(instance_->session_!=NULL){
			instance_->session_->ReceivedMessage(addr_mess);
		}
        std::vector<std::string> split = SplitString(std::string(addr_mess), std::string(":"), 2);
	    char *addr = new char[split[0].size()+1];
        char *mess = new char[split[1].size()+1];
	    std::strcpy(addr, split[0].c_str());
        std::strcpy(mess, split[1].c_str());        

        instance_->chatSessions_[std::string(addr)]->ReceivedMessage(mess);

        delete[] addr;
        delete[] mess;
	}

    void CommunicationUIModule::clearContactList()
    {
        this->lstContacts.lstContactsTreeModel->clear();
    }

	void CommunicationUIModule::contactReceived(char* t)
	{
		LogInfo("contact item");
        LogInfo(t);
		instance_->addContactItem(t);
	}

	//void CommunicationUIModule::addContactItem(char *contactID)
    void CommunicationUIModule::addContactItem(char *id_and_address)
	{
		//std::string str = "CGetContactWithID";
		//std::string syntax = "s";
		////!!
		//char** args = new char*[1];
		//char* buf1 = new char[20];
		//strcpy(buf1, contactID);
		//args[0] = buf1;

		//Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, args);
		//char* name = ret->ConvertToChar();

        std::vector<std::string> split = SplitString(std::string(id_and_address), std::string(":"), 2);
	    char *id = new char[split[0].size()+1];
        char *contact = new char[split[1].size()+1];
	    std::strcpy(id, split[0].c_str());
        std::strcpy(contact, split[1].c_str());        
        
        //this->lstContacts.columns_
        Gtk::TreeModel::Row row = *(this->lstContacts.lstContactsTreeModel->append());

        //std::string id(contactID);
        //std::string contact(name);
        row[this->lstContacts.columns_.id_] = std::string(id);
        row[this->lstContacts.columns_.contact_] = std::string(contact);
        row[this->lstContacts.columns_.status_] = "offline";
        row[this->lstContacts.columns_.message_] = "-";

        delete[] id;
        delete[] contact;
	}


   
    void CommunicationUIModule::OnContactListClicked()
    {
        LogInfo("OnContactListClicked");
    }
    
    void CommunicationUIModule::contactStatusChanged(char* id_n_status)
    {
        LogInfo("status change");
        LogInfo(id_n_status);
		//Foundation::ScriptObject* ret = instance_->imScriptObject->CallMethod(str, syntax, args);
        
        std::vector<std::string> split = SplitString(std::string(id_n_status), std::string(":"), 3);
        ////const char* id = split[0].c_str();
        //std::string id_str = split[0];
        //const char* status = split[1].c_str();

	    // allocate space for a zero-terminated copy of the string
	    char *id = new char[split[0].size()+1];
        char *status = new char[split[1].size()+1];
        char *status_string = new char[split[2].size()+1];
	    std::strcpy(id, split[0].c_str());
        std::strcpy(status, split[1].c_str());
        std::strcpy(status_string, split[2].c_str());
        LogInfo(id);
        
        if(strcmp(id, "1")==0){
            LogInfo("set on status");
            Glib::ustring title("Comms UI | (");
            title.append(status);
            title.append(" - ");
            title.append(status_string);
            title.append(")");
            // does not work
            LogInfo("set title");
            instance_->wndCommMain->set_title(title);
            //lblOnlineStatus
            //Gtk::Label* lblPresenceStatus;
            LogInfo("set label");
            instance_->lblPresenceStatus->set_text(title);
        } else {
    	    instance_->lstContacts.setContactStatus(id, status, status_string);
        }
	    delete[] id;
        delete[] status;
        delete[] status_string;
    }

    void CommunicationUIModule::handleAvailableStatusList(char* statuslist_N)
    {
        LogInfo("handleAvailableStatusList");
        LogInfo(statuslist_N);
        std::vector<std::string> split = SplitString(std::string(statuslist_N), std::string(":"), 0);
        for(std::vector<std::string>::iterator iter = split.begin();iter < split.end(); iter++){
            if(*iter!="unknown"&&*iter!="offline"){
                instance_->cmbPresence.append_text(*iter);
            }
        }
        
    }
    
    void CommunicationUIModule::handleRegisteringSuccess(char* reason)
    {
        LogInfo("handleRegisteringSuccess");
        std::string infoText = "Registering succeeded: ";
        infoText.append(reason);
        SelectionDialog sd("Registering succeeded", infoText , "_Ok", "_Cancel", instance_, false);
        Gtk::Main::run(sd);
    }

    void CommunicationUIModule::handleRegisteringFailure(char* reason)
    {
        LogInfo("handleRegisteringFailure");
        std::string infoText = "Registering failed: ";
        infoText.append(reason);
        SelectionDialog sd("Registering failed", infoText , "_Ok", "_Cancel", instance_, false);
        Gtk::Main::run(sd);        
    }

    void CommunicationUIModule::contactAdded(char* addr)
    {
        LogInfo("contactAdded");
        std::string infoText = "user ";
        infoText.append(addr);
        infoText.append(" accepted your contact request");
        SelectionDialog sd("Contact accepted", infoText , "_Ok", "_Cancel", instance_, std::string(addr));
        Gtk::Main::run(sd);
    }

    void CommunicationUIModule::contactAddedToPublishList(char* addr)
    {
        LogInfo("contactAddedToPublishList");
        std::string infoText = "user ";
        infoText.append(addr);
        infoText.append(" added to your publish list, subscribe?");
        SelectionDialog sd("Subscribe incoming", infoText , "_Ok", "_Cancel", instance_, std::string(addr));
        Gtk::Main::run(sd);        
    }

    void CommunicationUIModule::contactRemoved(char* id)
    {
        LogInfo("contactRemoved"); 
        instance_->lstContacts.RemoveContact(id);
        //sprintf(
        std::string infoText = "User ";
        infoText.append(id);
        infoText.append(" removed you from his/hers contact list");
        SelectionDialog sd("Contact removed", infoText, "_Ok", "_Not shown", instance_, std::string(id), false);
        Gtk::Main::run(sd);
    }
    void CommunicationUIModule::remotePending(char* id)
    {
        LogInfo("remotePending"); 
        LogInfo(id); 
        std::string infoText = "Let user ";
        infoText.append(id);
        infoText.append(" see your presence status");

        SelectionDialog sd("Remote request", infoText, "_Allow", "_Deny", instance_, std::string(id));
        Gtk::Main::run(sd);
    }
    void CommunicationUIModule::localPending(char* id)
    {
        LogDebug("localPending"); 
        LogInfo(id); 
        std::string infoText = "Let user ";
        infoText.append(id);
        infoText.append(" see your presence status");

        SelectionDialog sd("Presence request", infoText, "_Allow", "_Deny", instance_, std::string(id));
        Gtk::Main::run(sd);
    }

    void CommunicationUIModule::incomingRequest(char* addr)
    {
        LogInfo("incomingRequest"); 
        LogInfo(addr); 
        std::string infoText = "User ";
        infoText.append(addr);
        infoText.append(" want's to add you as a contact");
        SelectionDialog sd("Incoming contact request", infoText, "_Allow", "_Deny", instance_, std::string(addr));
        Gtk::Main::run(sd);
    }

    char** CommunicationUIModule::buildOneStringParamArrayFromConstCharArray(const char* prm)
    {
        size_t len = strlen(prm);
		char* buf1 = new char[strlen(prm)+1];
		strcpy(buf1, prm);
        char** args = new char*[1];
        args[0] = buf1;
		return args;
    }

    Foundation::ScriptObject* CommunicationUIModule::CallIMPyMethod(char* method, char* syntax, std::string& param)
    {
        if(strcmp(syntax, "")!=0){
            char** args = buildOneStringParamArrayFromConstCharArray(param.c_str());
            return instance_->imScriptObject->CallMethod(std::string(method), std::string(syntax), args); // memory freed in pymod
            //\bug need to know how actually ret values from python scripts are freed            
        } else {
            return instance_->imScriptObject->CallMethod(std::string(method), std::string(syntax), NULL); // memory freed in pymod
            //\bug need to know how actually ret values from python scripts are freed            
        }
    }
    
    std::vector<std::string> CommunicationUIModule::SplitString(const std::string &inString, const std::string &separator, const int &splitAmount)
    {
       std::vector<std::string> returnVector;
       std::string::size_type start = 0;
       std::string::size_type end = 0;
       int round = 0;
       if(splitAmount!=0){
           while ((end=inString.find (separator, start)) != std::string::npos && round < splitAmount)
           {
              returnVector.push_back (inString.substr (start, end-start));
              start = end+separator.size();
              round++;
           }
       } else {
           while ((end=inString.find (separator, start)) != std::string::npos)
           {
              returnVector.push_back (inString.substr (start, end-start));
              start = end+separator.size();
              round++;
           }       
       }
       returnVector.push_back (inString.substr (start));

       return returnVector;
    } 

	/*
 	 * Handles all communication events
     */
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

				std::string map_key = m->GetAuthor()->GetContact()->GetContactInfo("jabber")->GetProperty("address"); // <--- THIS WORKS ONLY IF THERE IS ONLY ONE PARTICIPANT PER SESSION !!!!
				chatSessions_[map_key]->OnMessageReceived(m);

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
                Gtk::Label* label = (Gtk::Label*)instance_->commUI_XML->get_widget("lblOnlineStatus");
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
						// TODO: FIX THIS!!! We have to have an another way to select right session!
						std::string map_key = s->GetParticipants()->at(0)->GetContact()->GetContactInfo("jabber")->GetProperty("address");
						chatSessions_[map_key]->OnStateChanged();
					}
					break;
				}

			}
		}
       
        return false;
    }    


	void CommunicationUIModule::UpdateOnlineStatusList()
	{
		std::vector<std::string> options = communication_service_->GetPresenceStatus()->GetOnlineStatusOptions();
//        std::vector<std::string> split = SplitString(std::string(statuslist_N), std::string(":"), 0);
        for(std::vector<std::string>::iterator iter = options.begin(); iter < options.end(); iter++)
		{
			std::string option = (*iter);
			if ( option.compare("unknown") == 0 || option.compare("offline") == 0)
				continue;
            instance_->cmbPresence.append_text(option);
        }
	}

	/*
	 * Refresh contact list element
	 */
	void CommunicationUIModule::UpdateContactList()
	{
		// todo fill ui contact list with content of this->contact_list_
		std::string protocol = "jabber"; // todo: remove this fixed definition

		contact_list_ = communication_service_->GetContactList();
		this->lstContacts.lstContactsTreeModel->clear();
		for(Communication::ContactList::iterator i = contact_list_->begin(); i < contact_list_->end(); i++)
		{
			Gtk::TreeModel::Row row = *(this->lstContacts.lstContactsTreeModel->append());
			Communication::ContactPtr c = (*i);
			//std::string id(contactID);
			//std::string contact(name);

			row[this->lstContacts.columns_.id_] = ""; // std::string(id);
			row[this->lstContacts.columns_.contact_] = c->GetContactInfo(protocol)->GetProperty("address");
			row[this->lstContacts.columns_.status_] = c->GetPresenceStatus()->GetOnlineStatus();
			row[this->lstContacts.columns_.message_] = c->GetPresenceStatus()->GetOnlineMessage();
		}
	}

	void CommunicationUIModule::HandleIncomingIMSession(Communication::IMSessionPtr session)
	{

	}

}


using namespace CommunicationUI;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationUIModule)
POCO_END_MANIFEST
