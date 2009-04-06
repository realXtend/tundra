#pragma once

//#include <stdafx.h>

class SetAccountDlg
{
public:
	SetAccountDlg(Gtk::Window *window);
	~SetAccountDlg(void);

	void RunAccountDlg(std::map<std::string, Foundation::Comms::SettingsAttribute> attributes);

private:
	Gtk::Window *dlgAccount;
};
