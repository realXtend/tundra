// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <boost/tokenizer.hpp>
#include "Native.h"
#include "ConsoleModule.h"
    

namespace Console
{
    void NativeInput::operator()()
    {
        assert (command_service_);

        while (true)
        {
            boost::this_thread::interruption_point();
            
            std::string command_line;
            std::getline(std::cin, command_line);

            command_service_->QueueCommand(command_line);
        }
    }


    // ***********************************************************


    Native::Native(Console::ConsoleCommandServiceInterface *command_service) : Console::ConsoleServiceInterface()
    {
        assert (command_service);
        input_.SetCommandManager(command_service);
        thread_ = boost::thread(boost::ref(input_));
    }

    Native::~Native()
    {
    }
}

