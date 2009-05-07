#ifndef incl_SelectionDlg_h
#define incl_SelectionDlg_h

#pragma once

#pragma warning( push )
#pragma warning( disable : 4250 )





namespace CommunicationUI
{

    class SelectionDialog : public Gtk::Dialog
    {
        
    public:
        SelectionDialog(std::string name, 
                        std::string question, 
                        std::string okLabel,
                        std::string cancelLabel,
                        DialogCallBackInterface* dialogCaller,
                        bool cancelEnabled = true);

        SelectionDialog(std::string name, 
                        std::string question, 
                        std::string okLabel,
                        std::string cancelLabel,
                        DialogCallBackInterface* dialogCaller,
                        std::string passData,
                        bool cancelEnabled = true);

        void Initialize();

        virtual ~SelectionDialog(void);

    protected:

		void onEntryDlgOk();
		void onEntryDlgCancel();
        void callback(char *);
        void cleanUp();

        std::string name_;
        std::string question_;
        std::string okLabel_;
        std::string cancelLabel_;
        bool usePassData_;
        std::string passData_;
        bool cancelEnabled_;

	    Gtk::Button btnOk_;
	    Gtk::Button btnCancel_;
        //Gtk::Button btnClose_;
        Gtk::Label lblQuestion_;

        DialogCallBackInterface* dialogCaller_;
        //DialogCallBackInterfacePtr dialogCaller_;
    };
}
#pragma warning( pop )
#endif
