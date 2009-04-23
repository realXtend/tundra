#ifndef incl_ContactList_h
#define incl_ContactList_h

#pragma once

#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm.h>

namespace Communication
{

    /* Can't use glade here 'cause need to override on_button_press_event */
    class ContactList : public Gtk::TreeView
	{
	public:
		ContactList();
		virtual ~ContactList(void);
    protected:
        Gtk::Menu popContactMenu;
        //Signal handlers:
        virtual bool on_button_press_event(GdkEventButton* event);

        //void onPopUpMenuEvent(GdkEventButton* event);
   
        virtual void on_menu_file_popup_generic();


		
	};
}

#endif