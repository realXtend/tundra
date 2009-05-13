

#ifndef incl_DialogCallbackInterface_h
#define incl_DialogCallbackInterface_h

#include "CommunicationUIModule.h"

namespace CommunicationUI
{
    class DialogCallBackInterface;
    typedef boost::shared_ptr<DialogCallBackInterface> DialogCallBackInterfacePtr;

	struct SettingsAttribute;

    class DialogCallBackInterface
    {
    public:
        DialogCallBackInterface()  {}
        virtual ~DialogCallBackInterface() {}
		virtual void Callback(std::string aConfigName, std::map<std::string, CommunicationUI::SettingsAttribute> attributes) = 0;
    };
}

#endif