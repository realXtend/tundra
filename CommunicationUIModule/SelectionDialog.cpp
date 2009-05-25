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

#include <gtkmm.h>

#include <Poco/ClassLibrary.h>

#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationUIModule.h"
#include <iterator>


#include "DialogCallbackInterface.h"
#include "SelectionDialog.h"



namespace CommunicationUI
{
     SelectionDialog::SelectionDialog(std::string name, 
                                      std::string question, 
                                      std::string okLabel,
                                      std::string cancelLabel,
                                      DialogCallBackInterface* aConfCaller,
                                      std::string passData,
                                      bool cancelEnabled)
                                      : name_(name), question_(question), okLabel_(okLabel), cancelLabel_(cancelLabel),
                                        btnOk_(okLabel, true), btnCancel_(cancelLabel, true), lblQuestion_(question, false), 
                                        passData_(passData), cancelEnabled_(cancelEnabled)
    {
        dialogCaller_ = aConfCaller;
        std::cout << "SelectionDialog constructor:" << std::endl;
        std::cout << "passData_ " << passData_ << std::endl;
        usePassData_ = true;
        Initialize();
    }


    SelectionDialog::SelectionDialog(std::string name, 
                                     std::string question, 
                                     std::string okLabel, 
                                     std::string cancelLabel,
                                     DialogCallBackInterface* aConfCaller,
                                     bool cancelEnabled) 
                                     : name_(name), question_(question), okLabel_(okLabel), cancelLabel_(cancelLabel),
                                       btnOk_(okLabel, true), btnCancel_(cancelLabel, true), lblQuestion_(question, false), 
                                       cancelEnabled_(cancelEnabled)
    {
        dialogCaller_ = aConfCaller;
        usePassData_ = false;
        Initialize();
    }



    void SelectionDialog::Initialize(){

        set_title(name_);
	    set_border_width(0);
	    set_size_request(300, 175);
	    get_vbox()->pack_start(lblQuestion_);
	    Gtk::Box* pBox = get_action_area();
        if(pBox){
	        pBox->pack_start(btnOk_);
            if(cancelEnabled_){
	            pBox->pack_start(btnCancel_);
            }
        }
        btnOk_.signal_clicked().connect( sigc::mem_fun(*this, &SelectionDialog::onEntryDlgOk));
        btnCancel_.signal_clicked().connect( sigc::mem_fun(*this, &SelectionDialog::onEntryDlgCancel));

        //btnOk_.grab_default();
	    show_all_children();

    }

    SelectionDialog::~SelectionDialog(void)
    {
        
    }

    void SelectionDialog::onEntryDlgOk()
    {
        callback("Ok");
    }
    void SelectionDialog::onEntryDlgCancel()
    {
        callback("Cancel");
    }

    void SelectionDialog::callback(char *answer)
    {
	    std::map<std::string, SettingsAttribute> attributes;
	    SettingsAttribute attr;

		attr.type = CommSettingsType::String;
        if(usePassData_){
            std::cout << "using passdata" << passData_ << std::endl;
            attr.value = passData_;
        } else {
            std::cout << "not using passdata" << std::endl;
            attr.value = "";
        }
         
        std::cout << "passdata" << std::endl;
        std::cout << "attr.value" << attr.value << std::endl;
	    attributes[std::string(answer)] = attr;
        
	    // calling back
        this->dialogCaller_->Callback(this->name_, attributes);
        cleanUp();
    }

    void SelectionDialog::cleanUp()
    {
	    hide();
    }

}