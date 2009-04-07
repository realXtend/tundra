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
#include "CommunicationUIModule.h"
#include <iterator>

#include "ConfigureDlg.h"



ConfigureDlg::ConfigureDlg(int count, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes, std::string name,
							 IConfigureCallBack* aConfCaller)
: m_Table(count, 2),
  m_Button_Close("_Close", true)
{
	set_title(name);
	configName = name;
	set_border_width(0);
	set_size_request(300, count*75);
	
	configCallback = aConfCaller;
	//commManager = aCommManager;
	//this->callbackMethod = callback;
	


	m_ScrolledWindow.set_border_width(10);

	//m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

	get_vbox()->pack_start(m_ScrolledWindow);
	m_Table.set_row_spacings(2);
	m_Table.set_col_spacings(10);

	/* pack the table into the scrolled window */
	m_ScrolledWindow.add(m_Table);

	std::map<std::string, Foundation::Comms::SettingsAttribute>::const_iterator iter;	

	int row = 0;
	for(iter = attributes.begin(); iter!=attributes.end(); ++iter)
	{
		// todo: check for string lenght
		char buffer[32];
		strcpy(buffer, iter->first.c_str());
		strcat(buffer, ":");
		Gtk::Label* pLabel = Gtk::manage(new Gtk::Label(buffer));
		pLabel->set_justify(Gtk::Justification::JUSTIFY_RIGHT);
		m_Table.attach(*pLabel, 0, 1, row, row + 1);
		Gtk::Entry* pEntry = Gtk::manage(new Gtk::Entry());
		m_Table.attach(*pEntry, 1, 2, row, row + 1);

		accessWidgets* pair = new accessWidgets();
		pair->label = pLabel;
		pair->entry = pEntry;
		widgetPairs.push_back(pair);
		
		row++;
	}
	



	/* Add a "close" button to the bottom of the dialog */
	m_Button_Close.signal_clicked().connect( sigc::mem_fun(*this,
			  &ConfigureDlg::on_button_close));

	/* this makes it so the button is the default. */
	m_Button_Close.set_flags(Gtk::CAN_DEFAULT);

	Gtk::Box* pBox = get_action_area();
	if(pBox)
	pBox->pack_start(m_Button_Close);

	/* This grabs this button to be the default button. Simply hitting
	* the "Enter" key will cause this button to activate. */
	m_Button_Close.grab_default();

	show_all_children();
}

ConfigureDlg::~ConfigureDlg()
{
}

void ConfigureDlg::on_button_close()
{
	// read values to map:
	std::map<std::string, Foundation::Comms::SettingsAttribute> attributes;
	
	std::vector<accessWidgets*>::iterator iter;
	for(iter = widgetPairs.begin();iter!=widgetPairs.end();iter++)
	{
		accessWidgets* aW = *iter;
		std::string name = aW->label->get_text().raw();
		// remove ':'
		name = name.substr(0, name.size()-1);
		std::string value = aW->entry->get_text().raw();
		Communication::CommunicationUIModule::LogInfo(name);
		Communication::CommunicationUIModule::LogInfo(value);
		Foundation::Comms::SettingsAttribute attr;
		attr.type = Foundation::Comms::String;
		attr.value = value;
		attributes[name] = attr;
	}

	// set configs
	configCallback->Callback(configName, attributes);
	//commManager->SetAccountAttributes(attributes);


	//Gtk::Table::TableList::iterator iter;
	//Gtk::Table::TableList l = m_Table.children();	
	//
	//for(iter = l.begin();iter!=l.end();++iter)
	//{
	//	//guint left = iter->get_left_attach();
	//	Gtk::Widget* w = iter->get_widget();
	//	GtkLabel* label = GTK_LABEL(w);
	//	gchar* t = label->text;
	//	std::string attrName(t);
	//	
	//	//Gtk::Label* label = GTK::LABEL(w);
	//	//if(left==0){
	//	//Gtk::Label *label = dynamic_cast<Gtk::Widget> w;
	//	//Gtk::Label label = (Gtk::Label)*w;
	//	//Glib::ustring text = label.get_text();
	//	//std::string attrName = text.raw();
	//	++iter;
	//	w = iter->get_widget();
	//	GtkEntry* entry = GTK_ENTRY(w);
	//	t = entry->text;
	//	std::string attrValue(t);
	//	//} else {
	//	//Gtk::Entry *entry = (Gtk::Entry)w;
	//	//attr.value = entry.get_text().raw();
	//	//}
	//	Foundation::Comms::SettingsAttribute attr;
	//	attr.type = Foundation::Comms::String;
	//	attributes[attrName]=attr;
	//}
		
	//delete allocated pairs
	for(iter = widgetPairs.begin();iter!=widgetPairs.end();iter++)
	{
		accessWidgets* aW = *iter;
		delete aW;
	}		
	widgetPairs.clear();

	hide();
}
