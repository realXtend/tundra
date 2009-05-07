#ifndef incl_SetConfigureDlg_h
#define incl_SetConfigureDlg_h

//#include <stdafx.h>

#include "DialogCallbackInterface.h"

#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm.h>

namespace CommunicationUI
{

    class IConfigureCallBack
    {
    public:
        IConfigureCallBack()  {}
        virtual ~IConfigureCallBack() {}
	    virtual void Callback(std::string aConfigName, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes) = 0;
    };


    class ConfigureDlg : public Gtk::Dialog
    {
    public:
	    ConfigureDlg(int count, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes, std::string name,
		   DialogCallBackInterface* aConfCaller);

      //  ConfigureDlg(int count, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes, std::string name,
		    //DialogCallBackInterfacePtr aConfCaller);

	    virtual ~ConfigureDlg(void);

    protected:
	    virtual void onButtonClose();
	    virtual void onButtonOk();
	    virtual void onButtonCancel();

	    void cleanUp();

        //Child widgets:
	    Gtk::ScrolledWindow m_ScrolledWindow;
	    Gtk::Table m_Table;
	    Gtk::Button m_Button_Close;
	    Gtk::Button m_Button_Ok;
	    Gtk::Button m_Button_Cancel;

	    struct accessWidgets
	    {
		    Gtk::Label* label;
		    Gtk::Entry* entry;
	    };

	    // MVC model part
	    // did not know how to read widgets from table, so adding references to all entries for reading
	    std::vector<accessWidgets*> widgetPairs;

	    std::string configName;
	    //IConfigureCallBack* configCallback;
        DialogCallBackInterface* configCallback;
        //DialogCallBackInterfacePtr configCallback;

	    //Foundation::Comms::CommunicationManagerServiceInterface* commManager;
	    void (*callbackMethod)(std::map<std::string, Foundation::Comms::SettingsAttribute>);  
    };
}
#pragma warning( pop )
#endif
