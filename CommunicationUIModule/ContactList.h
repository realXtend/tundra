#ifndef incl_ContactList_h
#define incl_ContactList_h

#pragma once

#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm.h>




namespace CommunicationUI
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
            ModelColumns(){ add(id_); add(contact_); add(status_); add(message_);}

			Gtk::TreeModelColumn<std::string> id_;
			Gtk::TreeModelColumn<std::string> contact_;
            Gtk::TreeModelColumn<std::string> status_;
            Gtk::TreeModelColumn<std::string> message_;
		};

        Glib::RefPtr<Gtk::ListStore> lstContactsTreeModel;
        ModelColumns columns_;


        Gtk::TreeModel::iterator GetSelected();
        void RemoveContact(char* id);
        Gtk::TreeModel::iterator getRowWithId(char* id);

        void setContactStatus(char* id, char* status, char* status_string);

    protected:
        //Signal handlers:
        virtual bool on_button_press_event(GdkEventButton *event);
        virtual bool on_key_press_event(GdkEventKey *event);
        void startChat();
        void startVoip();
        void removeContact();
        bool notFound;

        Gtk::Menu popContactMenu;
        
        CommunicationUIModule* uimodule_;
		Communication::ContactInfoListPtr contact_list_;
	};
}

#endif