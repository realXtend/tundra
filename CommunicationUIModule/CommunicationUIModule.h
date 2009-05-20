
#ifndef incl_CommunicationUIModule_h
#define incl_CommunicationUIModule_h

#pragma once

#pragma warning( push )
#pragma warning( disable : 4275 ) // IConfigureCallBack used only inside CommunicationUIModule 
#include "Foundation.h"

#include "ChatSession.h"
#include "ConfigureDlg.h"
#include "DialogCallbackInterface.h"

#include "ModuleInterface.h"
#include "ContactList.h"

namespace CommunicationUI
{

	enum CommSettingsType 
	{
		String = 0,
		Int,
		Boolean
	};

	struct SettingsAttribute
	{
		//std::string name;
		CommSettingsType type;
		std::string value;
		int length;
	};

	typedef boost::shared_ptr<ChatSession> ChatSessionUIPtr;
	//
	//class MODULE_API CommunicationUIModule : public Foundation::ModuleInterfaceImpl, public IConfigureCallBack
    class MODULE_API CommunicationUIModule : public Foundation::ModuleInterfaceImpl, public CommunicationUI::DialogCallBackInterface
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

		virtual void Callback(std::string aConfigName, std::map<std::string, SettingsAttribute> attributes);

		MODULE_LOGGING_FUNCTIONS
		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_CommunicationUI;

        virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);
		Core::event_category_id_t communication_event_category_id_;

	private:
		void initializeMainCommWindow();
		void SetupCommunicationServiceUsage();
		void OnAccountMenuSettings();
        void OnCreateAccount();
		void OnAccountMenuSetAccountAndPassword();
		void OnAccountMenuConnect();
		void OnAccountMenuDisconnect();
		void OnDirectChatMenuStartChat();

        void OnSetStatusOnline();
        void OnSetStatusAway();
        void OnSetStatusBusy();
        void OnSetStatusOffline();

        void OnContactListClicked();
        void OnSetPresenceMessage();

		void OnEntryDlgOk();
		void OnEntryDlgCancel();

		void OnContactAdd();
		void OnContactRemove();
        void OnRefresh();
		void OnPresenceStatusSelection();

		void UpdateOnlineStatusList();
		void UpdateContactList();
		Communication::ContactPtr FindContact(std::string address);
		void HandleIncomingIMSession(Communication::IMSessionPtr session);
		ChatSessionUIPtr GetUIChatSession(Communication::IMSessionPtr s);
		ChatSessionUIPtr CreateUIChatSession(Communication::IMSessionPtr s);
		void HandleFriendRequest(Communication::FriendRequestPtr r);
		void ShowFriendRequestWindow(Communication::FriendRequestPtr r);
		Communication::FriendRequestPtr	FindFriendRequest(std::string address);
		void SaveCredentials(Communication::CredentialsPtr c);
		Communication::CredentialsPtr LoadCreadentials();

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
        Gtk::Button* btnAddContact;
        Gtk::Button* btnRemoveContact;
        Gtk::Button* btnSetPresenceMessage;
        Gtk::Entry* entryPresenceMessage;

        //Gtk::ComboBox* cmbPresence;
        Gtk::ComboBoxText cmbPresence;
        Gtk::Label* lblPresenceStatus;
        
	protected:

		int entryret_;
		bool online_status_list_ready_; 
			
	public:
//		ChatSessionUIPtr session_;
	    std::map<std::string, ChatSessionUIPtr> chatSessions_; // todo: do we need a map here?

	private:
		// Service References
		Communication::CommunicationServicePtr communication_service_;

		Communication::ContactListPtr contact_list_;
		typedef std::map<std::string, Communication::FriendRequestPtr> FriendRequestMap;
		FriendRequestMap friend_requests_;
	};
}
#pragma warning( pop )
#endif
