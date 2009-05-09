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

namespace CommunicationUI
{


    //ContactList::ContactList(CommunicationUIModule& uimodule): uimodule_(uimodule)
    ContactList::ContactList()
    {
        //Fill popup menu:
        Gtk::Menu::MenuList& menulist = popContactMenu.items();

        menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Chat",
            sigc::mem_fun(*this, &ContactList::startChat) ) );
        menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Voip",
            sigc::mem_fun(*this, &ContactList::startVoip) ) );
        menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Remove",
            sigc::mem_fun(*this, &ContactList::removeContact) ) );

        lstContactsTreeModel = Gtk::ListStore::create(columns_);
        this->set_model(lstContactsTreeModel);

        this->append_column("ID", this->columns_.id_);
		this->append_column("Contact", columns_.contact_);
        this->append_column("Status", columns_.status_);
        this->append_column("Message", columns_.message_);
        notFound = false;

        //Gtk::TreeModel::Row row = *(lstContactsTreeModel->append());
        //row[columns_.id_] = "test";
        //row[columns_.contact_] = "test";

    }

    ContactList::~ContactList(void)
    {
    }

    void ContactList::SetModule(CommunicationUIModule& uimodule){ 
        this->uimodule_ = &uimodule;
    }

    bool ContactList::on_button_press_event(GdkEventButton* event)
    {
        CommunicationUI::CommunicationUIModule::LogInfo("ContactList clicked!!");
        bool return_value = TreeView::on_button_press_event(event);
        if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
        {
            popContactMenu.popup(event->button, event->time);
        }
        return return_value;
    }


    
    bool ContactList::on_key_press_event(GdkEventKey *event)
    {
        
        bool handled = false;
        switch (event->keyval)
        {
            case GDK_Return:
            case GDK_KP_Enter:
            case GDK_ISO_Enter:
            case GDK_3270_Enter:
                startChat();
                //event->time
                //gtk_main_do_event(
                //popContactMenu.popup(
                //Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();
                //if(refSelection)
                //{
                //    refSelection->
                //}
                handled = true;
                break;
            case GDK_Tab:
                //Communication::CommunicationUIModule::LogDebug("TAB:");
                handled = true;
                break;
        }
        if(!handled){ handled = Gtk::TreeView::on_key_press_event(event); }
        return handled;
    }

    void ContactList::startChat()
    {
        //Communication::CommunicationUIModule::LogDebug("onPopUpMenuEvent!!");
        Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();
        if(refSelection)
        {
            Gtk::TreeModel::iterator iter = refSelection->get_selected();
            if(iter)
            {
                //iter->get_value
                std::string id = (*iter)[this->columns_.id_];
                std::string contact = (*iter)[this->columns_.contact_];
                //int id = (*iter)[m_Columns.m_col_id];

                this->uimodule_->StartChat(contact.c_str());
            }
        }
    }
    Gtk::TreeModel::iterator ContactList::GetSelected(){
        //Communication::CommunicationUIModule::LogDebug("GetSelected");
        Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();
        return refSelection->get_selected();
        //if(refSelection)
        //{
        //    Gtk::TreeModel::iterator iter = 
        //    return iter;
        //}
        //return ;
    }

    void ContactList::startVoip(){
        
    }
    void ContactList::removeContact(){
        
    }
    void ContactList::RemoveContact(char* id){
        //Communication::CommunicationUIModule::LogInfo("RemoveContact");
        Gtk::TreeModel::iterator iter = getRowWithId(id);

        if(!notFound){ 
            //Communication::CommunicationUIModule::LogInfo("row to remove was found");
            lstContactsTreeModel->erase(iter);
        } else {
            notFound = false;
            //Communication::CommunicationUIModule::LogInfo("row to remove was not found:");
            //Communication::CommunicationUIModule::LogInfo(id);
        }
    }

    void ContactList::setContactStatus(char* id, char* status, char* status_string)
    {
        Gtk::TreeModel::iterator iter = getRowWithId(id);
        if(!notFound){ 
            Gtk::TreeModel::Row row = *iter;
            row.set_value(2, std::string(status));
            row.set_value(3, std::string(status_string));
            //row.get_value(
        } else {
            notFound = false;
        }

        //Gtk::TreeModel::Children children = lstContactsTreeModel->children();
        //for(Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++iter)
        //{            
        //    Gtk::TreeModel::Row row = *iter;
        //    std::string val;
        //    row.get_value(0, val);
        //    if(val==std::string(id))
        //    {
        //        row.set_value(2, std::string(status));    
        //    }
        //}
    }

    Gtk::TreeModel::iterator ContactList::getRowWithId(char* id)
    {
        Gtk::TreeModel::Children children = lstContactsTreeModel->children();
        for(Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++iter)
        {            
            Gtk::TreeModel::Row row = *iter;
            std::string val;
            row.get_value(0, val);
            if(val==std::string(id))
            {
                return iter;
            }
        }
        notFound = true;
        return children.begin(); // pointing to none?
    }
}
