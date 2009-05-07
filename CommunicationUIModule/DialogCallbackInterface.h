

#ifndef incl_DialogCallbackInterface_h
#define incl_DialogCallbackInterface_h


namespace CommunicationUI
{
    class DialogCallBackInterface;
    typedef boost::shared_ptr<DialogCallBackInterface> DialogCallBackInterfacePtr;

    class DialogCallBackInterface
    {
    public:
        DialogCallBackInterface()  {}
        virtual ~DialogCallBackInterface() {}
	    virtual void Callback(std::string aConfigName, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes) = 0;
    };
}

#endif