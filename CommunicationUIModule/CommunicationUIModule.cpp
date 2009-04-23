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
#include "ScriptServiceInterface.h"

#include "CommunicationUIModule.h"



namespace Communication
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
		commManager = framework_->GetService<Foundation::Comms::CommunicationManagerServiceInterface>(Foundation::Service::ST_CommunicationManager);
		scriptService = framework_->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting);
		std::string error;
		Foundation::ScriptObject* script = scriptService->LoadScript("IMDemo", error);
		if(error=="None")
		{
			this->imScriptObject = script->GetObject("IMDemo");
			std::string str = "CDoStartUp";
			std::string syntax = "";
			Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, NULL);
		}
		Foundation::ScriptEventInterface *eIntf = dynamic_cast<Foundation::ScriptEventInterface*>(this->scriptService);

		sessionUp_ = false;
		CommunicationUIModule::instance_= this;

		eIntf->SetCallback(CommunicationUIModule::testCallback, "key");
		eIntf->SetCallback(CommunicationUIModule::connected, "connected");
		eIntf->SetCallback(CommunicationUIModule::disconnected, "disconnected");
		eIntf->SetCallback(CommunicationUIModule::disconnected, "connecting");

		eIntf->SetCallback(CommunicationUIModule::channelOpened, "channel_opened");
		eIntf->SetCallback(CommunicationUIModule::channelClosed, "channel_closed");
		eIntf->SetCallback(CommunicationUIModule::messagReceived, "message_received");
		eIntf->SetCallback(CommunicationUIModule::friendReceived, "contact_item");
		
	}

	void CommunicationUIModule::Uninitialize()
	{
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
        if(!wndCommMain||!fxdContainer)
			return;

		lstBuddiesTreeModel = Gtk::ListStore::create(columns_);
        lstContactsTreeModel = Gtk::ListStore::create(columns2_);
        lstContacts.set_model(lstContactsTreeModel);
		lstContacts.append_column("ID", columns2_.id_);
		lstContacts.append_column("Contact", columns2_.contact_);
        lstContacts.set_size_request(219,82);
        fxdContainer->put(lstContacts, 27, 169);

		commUI_XML->get_widget("lstBuddies", lstBuddies);
		lstBuddies->set_model(lstBuddiesTreeModel);
        
		lstBuddies->append_column("ID", columns_.id_);
		lstBuddies->append_column("Contact", columns_.contact_);

        Gtk::TreeModel::Row row = *(lstContactsTreeModel->append());
        row[columns2_.id_] = "test";
        row[columns2_.contact_] = "test";


	    commUI_XML->connect_clicked("mi_connect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));
		commUI_XML->connect_clicked("mi_disconnect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuDisconnect));
		commUI_XML->connect_clicked("mi_setaccount", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuSetAccountAndPassword));
		commUI_XML->connect_clicked("mi_settings", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuSettings));
		commUI_XML->connect_clicked("mi_directchat", sigc::mem_fun(*this, &CommunicationUIModule::OnDirectChatMenuStartChat));
		commUI_XML->connect_clicked("btnTest", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));
        commUI_XML->connect_clicked("lstBuddies", sigc::mem_fun(*this, &CommunicationUIModule::OnContactListClicked));
        
		// entry dialog
		commUI_XML->connect_clicked("btnEntryOk", sigc::mem_fun(*this, &CommunicationUIModule::OnEntryDlgOk));
		commUI_XML->connect_clicked("btnEntryCancel", sigc::mem_fun(*this, &CommunicationUIModule::OnEntryDlgCancel));

        
        
		wndCommMain->show();
        lstContacts.show();
        //wndCommMain->show_all();
	}

	void CommunicationUIModule::OnAccountMenuSettings()
	{
		std::string error;
		
		//Foundation::ScriptObject* script = scriptService->LoadScript("pymodules/py_class3", error);
		Foundation::ScriptObject* script = scriptService->LoadScript("py_class3", error);
		
		if(error=="None"){
			LogInfo("got script object");
			
			sobj = script->GetObject("Multiply");
			std::string str = "multiply";
			std::string syntax = "";
			Foundation::ScriptObject* ret = sobj->CallMethod(str, syntax, NULL);
			LogInfo("Received: ");
			char* cret = ret->ConvertToChar();
			if(cret!=NULL){
				LogInfo(cret);
			} else {
				LogInfo("NULL");
			}

		} else {
			LogInfo("script loading failed");
		}
		//scriptService->RunString("Test.Run()");
	}

	void CommunicationUIModule::OnAccountMenuSetAccountAndPassword()
	{
		//LogInfo("Set account");
		std::map<std::string, Foundation::Comms::SettingsAttribute> attributes = commManager->GetAccountAttributes();
		int count = attributes.size();
		LogInfo(attributes.find("name")->first);

		//ConfigureDlg accDlg(count, attributes, "account settings", commManager, this);
		ConfigureDlg accDlg(count, attributes, "account settings", this);
		Gtk::Main::run(accDlg);
		//accDlg.
	}

	void CommunicationUIModule::OnAccountMenuConnect()
	{
		// TODO: Eventually this code would not be here as it is no UI specific
		std::string str = "CAccountConnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, NULL);
	}
	
	void CommunicationUIModule::OnAccountMenuDisconnect()
	{
		// TODO: Eventually this code would not be here as it is no UI specific
		std::string str = "CDisconnect";
		std::string syntax = "";
		Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, NULL);
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
			LogInfo("start chat window here");

			// Fix any memory leaks here !!

			char** args = new char*[2];
			char* buf1 = new char[20];
			strcpy(buf1,str.c_str());
			args[0] = buf1;

			std::string str = "CStartChatSession";
			std::string syntax = "s";
			Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, args);
            sessionUp_ = true;
			this->session_ = Communication::ChatSessionUIPtr(new Communication::ChatSession(str.c_str(), imScriptObject));
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


	void CommunicationUIModule::Callback(std::string aConfigName, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes)
	{
		if(aConfigName=="account settings"){ commManager->SetAccountAttributes(attributes); }
	}

	void CommunicationUIModule::setOnlineStatus(char* status)
	{
		//LogInfo("setting status");
		Gtk::Label* label = (Gtk::Label*)instance_->commUI_XML->get_widget("lblOnlineStatus");
		label->set_label(status);
	}

	void CommunicationUIModule::testCallback(char* t)
	{
		//std::cout << "testCallback" << std::endl;
		//std::cout << t << std::endl;
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
	}

	void CommunicationUIModule::channelOpened(char* t)
	{
		LogInfo("channelOpened");
		if(instance_->session_!=NULL&&instance_->sessionUp_!=false){
			instance_->session_->ChannelOpen();
		} else {
			instance_->session_ = Communication::ChatSessionUIPtr(new Communication::ChatSession(t, instance_->imScriptObject));
		}
		instance_->sessionUp_ = true;
	}

	void CommunicationUIModule::channelClosed(char* t)
	{
		LogInfo("channelClosed");
		if(instance_->session_!=NULL){
			instance_->session_->ChannelClosed();
		} 
		instance_->sessionUp_ = false;
	}

	void CommunicationUIModule::messagReceived(char* t)
	{
		if(instance_->session_!=NULL){
			instance_->session_->ReceivedMessage(t);
		}
	}

	void CommunicationUIModule::friendReceived(char* t)
	{
		LogInfo("contact item");
		instance_->addFriendItem(t);
	}

	void CommunicationUIModule::addFriendItem(char *contactID)
	{
		std::string str = "CGetFriendWithID";
		std::string syntax = "s";
		//!!
		char** args = new char*[1];
		char* buf1 = new char[20];
		strcpy(buf1, contactID);
		args[0] = buf1;

		Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, args);
		char* name = ret->ConvertToChar();
        Gtk::TreeModel::Row row = *(lstBuddiesTreeModel->append());

        std::string id(contactID);
        std::string contact(name);
        row[columns_.id_] = id;
        row[columns_.contact_] = contact;
		
	}

    void CommunicationUIModule::OnContactListClicked()
    {
        LogInfo("OnContactListClicked");
    }

}


using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationUIModule)
POCO_END_MANIFEST
