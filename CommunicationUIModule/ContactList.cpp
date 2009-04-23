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

#include <Poco/ClassLibrary.h>
#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationUIModule.h"
#include <iterator>

#include "ContactList.h"

namespace Communication
{


    ContactList::ContactList(void)
    {
        //Fill popup menu:
        Gtk::Menu::MenuList& menulist = popContactMenu.items();

        menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Chat",
        sigc::mem_fun(*this, &ContactList::on_menu_file_popup_generic) ) );
        menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Voip",
        sigc::mem_fun(*this, &ContactList::on_menu_file_popup_generic) ) );
        menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Remove",
        sigc::mem_fun(*this, &ContactList::on_menu_file_popup_generic) ) );
    }

    ContactList::~ContactList(void)
    {
    }

    bool ContactList::on_button_press_event(GdkEventButton* event)
    {
        Communication::CommunicationUIModule::LogInfo("ContactList clicked!!");
        bool return_value = TreeView::on_button_press_event(event);
        if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
        {
            popContactMenu.popup(event->button, event->time);
        }
        return return_value;
    }

    void ContactList::on_menu_file_popup_generic()
    {
        Communication::CommunicationUIModule::LogInfo("onPopUpMenuEvent!!");
    }

    //void ContactList::onPopUpMenuEvent(GdkEventButton* event){
    //    Communication::CommunicationUIModule::LogInfo("onPopUpMenuEvent!!");
    //}
}