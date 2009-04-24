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
        Communication::CommunicationUIModule::LogInfo("ContactList clicked!!");
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
                std::cout << "Enter: " << std::endl;
                handled = true;
                break;
            //case GDK_3270_Enter:
            //    std::cout << "Enter: " << std::endl;
            //    handled = true;
            //    break;
            case GDK_Tab:
                std::cout << "TAB: " << std::endl;
                handled = true;
                break;
        }
        if(!handled){ handled = Gtk::TreeView::on_key_press_event(event); }
        return handled;
    }

    void ContactList::startChat()
    {
       Communication::CommunicationUIModule::LogInfo("onPopUpMenuEvent!!");
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
                std::cout << "  Selected ID=" << id << std::endl;
                std::cout << "  Selected Contact=" << contact << std::endl;

                this->uimodule_->StartChat(contact.c_str());
                //std::cout << "  Selected ID=" << id << std::endl;
			    //char** args = new char*[2];
			    //char* buf1 = new char[20];
			    //strcpy(buf1,contact.c_str());
			    //args[0] = buf1;

			    //std::string str = "CStartChatSession";
			    //std::string syntax = "s";
			    //Foundation::ScriptObject* ret = imScriptObject->CallMethod(str, syntax, args);
       //         sessionUp_ = true;
			    //this->session_ = Communication::ChatSessionUIPtr(new Communication::ChatSession(contact.c_str(), imScriptObject));
            }
        }
    }
    void ContactList::startVoip(){
        
    }
    void ContactList::removeContact(){

    }

    //void ContactList::on_menu_file_popup_generic()
    //{
    //    Communication::CommunicationUIModule::LogInfo("onPopUpMenuEvent!!");
    //    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = get_selection();
    //    if(refSelection)
    //    {
    //        Gtk::TreeModel::iterator iter = refSelection->get_selected();
    //        if(iter)
    //        {
    //            //iter->get_value
    //            std::string id = (*iter)[this->columns_.id_];
    //            //int id = (*iter)[m_Columns.m_col_id];
    //            std::cout << "  Selected ID=" << id << std::endl;
    //            //std::cout << "  Selected ID=" << id << std::endl;
    //        }
    //    }
    //}

}