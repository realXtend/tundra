// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleNative_h
#define incl_ConsoleNative_h

#include "ConsoleServiceInterface.h"


namespace Foundation
{
    class Framework;
}

namespace Console
{
    //! Native debug console
    class Native : public Foundation::Console::ConsoleServiceInterface
    {
    public:
        //! default constructor
        Native();
        //! destructor
        virtual ~Native();

        //! Add a command to the debug console
        virtual void AddCommand(const Foundation::Console::Command &command);

        //! Parse and execute command line
        virtual void ExecuteCommand(const std::string &command_line);
    };
}

#endif

