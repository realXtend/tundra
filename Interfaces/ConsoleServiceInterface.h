// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ConsoleServiceInterface_h
#define incl_Interfaces_ConsoleServiceInterface_h

#include "ServiceInterface.h"

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
        virtual void Update(Core::f64 frametime) {}

        //! Prints text to the console
        virtual void Print(const std::string &text) = 0;

        //! scroll console text rel amount
        /*!
            \param rel positive to scroll up, negative to scroll down, 20 is one line.
        */
        virtual void Scroll(int rel) = 0;

        //! Hide / show console
        virtual void SetVisible(bool visible) = 0;

        //! Returns true if console is visible, false otherwise
        virtual bool IsVisible() const = 0;

        //! Is the console accepting input
        virtual bool IsActive() const = 0;

        //! event for key press
        virtual bool HandleKeyDown(int code, Core::uint text) = 0;
        //! event for key released
        virtual bool HandleKeyUp(int code, Core::uint text) = 0;
    };

    //! smart pointer for console services. \ingroup DebugConsole_group
    typedef boost::shared_ptr<ConsoleServiceInterface> ConsolePtr;
}

#endif

