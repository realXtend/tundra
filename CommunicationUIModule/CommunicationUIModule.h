
#ifndef incl_CommunicationUIModule_h
#define incl_CommunicationUIModule_h

#pragma once

#include "ChatSession.h"
#include "ConfigureDlg.h"
#include "ModuleInterface.h"
#include "ContactList.h"
//#include "PythonScriptModule.h"


// *********************************************'
// CommunicationViewModel<--->CommunicationView communication:
// 
// - Actions:
//   - ShowContactList(contact_list)
//   - HideContactList(contact_list)      needed ?
//   - ShowSessionInvitation(invitation)
//   - ShowMessageNotification(contact)
//   - ShowPresenceNotification(contact)
// 
// - data objects:
//   - ContactList
//   - Contact
//   - Session
//   - Participient
//
// *********************************************'


namespace Communication
{

	typedef boost::shared_ptr<ChatSession> ChatSessionUIPtr;
	//
	class MODULE_API CommunicationUIModule : public Foundation::ModuleInterfaceImpl, public IConfigureCallBack
	{
	public:
		CommunicationUIModule(void);
		virtual ~CommunicationUIModule(void);

		void Load();
		void Unload();
		void Initialize();
		void PostInitialize();
		void Uninitialize();
		void Update();

        void StartChat(const char* contact);

		virtual void Callback(std::string aConfigName, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes);

		MODULE_LOGGING_FUNCTIONS
		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_CommunicationUI;

	private:
		// event handlers
		static void testCallback(char*);
		static void connected(char*);
		static void connecting(char*);
		static void disconnected(char*);
		static void channelOpened(char*);
		static void channelClosed(char*);
		static void messagReceived(char*);
		static void friendReceived(char* t);
        static void contactStatusChanged(char* id);

		void setOnlineStatus(char* status);


	private:
		void initializeMainCommWindow();
		void OnAccountMenuSettings();
		void OnAccountMenuSetAccountAndPassword();
		void OnAccountMenuConnect();
		void OnAccountMenuDisconnect();
		void OnDirectChatMenuStartChat();
        void OnContactListClicked();

		void OnEntryDlgOk();
		void OnEntryDlgCancel();

		void addFriendItem(char *);


		Glib::RefPtr<Gnome::Glade::Xml> commUI_XML;

		// Widgets
		Gtk::Window *wndCommMain;
		Gtk::Window *dlgAccount;
		Gtk::ActionGroup *actionGroup;
		Gtk::Dialog* dlgEntry;
		//Gtk::TreeView* lstBuddies;
        Gtk::Menu* menuContactList;
        Gtk::Fixed* fxdContainer;
        ContactList lstContacts;

		//Glib::RefPtr<Gtk::ListStore> lstBuddiesTreeModel;
        //Glib::RefPtr<Gtk::ListStore> lstContactsTreeModel;


	protected:

		int entryret_;
			
		// Currently just 1 session
		bool sessionUp_;
	public:
		ChatSessionUIPtr session_;


	private:
		// Service References
		Foundation::Comms::CommunicationManagerServiceInterface *commManager;
		Foundation::ScriptServiceInterface *scriptService;

		// Scripts
		Foundation::ScriptObject* sobj;
		Foundation::ScriptObject* imScriptObject;

		// Pointer to ui instance (needed for accessing from static methods, which are needed for event passsing)
		static CommunicationUIModule* instance_;
		//Foundation::CommunicationUIManagerPtr CommunicationUI_manager_;

		std::map<std::string, std::string> contactList_;
	};
}

#endif