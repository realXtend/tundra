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



#include "SetAccountDlg.h"

SetAccountDlg::SetAccountDlg(Gtk::Window *window)
{
	dlgAccount = window;
}

SetAccountDlg::~SetAccountDlg(void)
{
	// make sure memory is freed
	if(dlgAccount!=NULL){
		delete dlgAccount;
		dlgAccount = NULL;
	}
}

void SetAccountDlg::RunAccountDlg(std::map<std::string, Foundation::Comms::SettingsAttribute> attributes)
{
	//dlgAccount->
}