// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ConsoleServiceInterface_h
#define incl_Interfaces_ConsoleServiceInterface_h

#include "ServiceInterface.h"
#include "CoreTypes.h"

namespace Console
{
    //! Interface for debug console service
    /*! Usually it is not necessary to use this service,
        the only really useful function is Print(), but the
        preferred way to output text to the console is via the
        log.

        Most functions are not threadsafe.

        \ingroup Services_group
        \ingroup DebugConsole_group
    */
    class ConsoleServiceInterface : public Foundation::ServiceInterface
    {
    public:
        //! default constructor
        ConsoleServiceInterface() {}

        //! destructor
        virtual ~ConsoleServiceInterface() {}

        //! add time
        virtual void Update(f64 frametime) {}

        //! Prints text to the console.
        /*! The text will be split to multiple lines to fit. '\n' should be used for line breaks.
        */
        virtual void Print(const std::string &text) = 0;

        //!Execute the specified command
        virtual void ExecuteCommand(const std::string &command) = 0;

        //! Hide / show console
        virtual void ToggleConsole() = 0;

        //! Sets Ui initialized/uninitialized
        virtual void SetUiInitialized(bool initialized)=0;

        //! Returns false if UI is not initialized, true otherwise
        virtual bool IsUiInitialized()=0;


    };

    //! smart pointer for console services. \ingroup DebugConsole_group
    typedef boost::shared_ptr<ConsoleServiceInterface> ConsolePtr;
}

#endif

