#ifndef incl_ContactList_h
#define incl_ContactList_h

#pragma once

#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm.h>




//class IChatStartCallBack
//{
//public:
//    IChatStartCallBack()  {}
//    virtual ~IChatStartCallBack() {}
//	virtual void StartChat(const char* contact) = 0;
//}

namespace Communication
{
    class CommunicationUIModule;

    /* Can't use glade here 'cause need to override on_button_press_event */
    class ContactList : public Gtk::TreeView
	{
	public:
		//ContactList(CommunicationUIModule& uimodule);
        ContactList();
		virtual ~ContactList(void);
    
        void SetModule(CommunicationUIModule& uimodule);
        
		class ModelColumns : public Gtk::TreeModel::ColumnRecord
		{
		public:
            ModelColumns(){ add(id_); add(contact_);}

			Gtk::TreeModelColumn<std::string> id_;
			Gtk::TreeModelColumn<std::string> contact_;
		};

    
        Glib::RefPtr<Gtk::ListStore> lstContactsTreeModel;
        ModelColumns columns_;

    protected:
        //Signal handlers:
        virtual bool on_button_press_event(GdkEventButton* event);
        //virtual void on_menu_file_popup_generic();
        void startChat();
        void startVoip();
        void removeContact();


        //Glib::RefPtr<Gtk::TextBuffer> buddyBuffer_;

        Gtk::Menu popContactMenu;
        
        CommunicationUIModule* uimodule_;
	};
}

#endif