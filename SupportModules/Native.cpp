// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "Native.h"
#include "ConsoleModule.h"
    

namespace Console
{
    void NativeInput::operator()()
    {
        assert (command_service_);

        while (true)
        {
            PROFILE(NativeInput);
            boost::this_thread::interruption_point();
            
            std::string command_line;
            std::getline(std::cin, command_line);

            if (std::cin.fail())
            {
                framework_->Exit();
                break;
            }

            PROFILE(NativeInput_QueueCommand);
            command_service_->QueueCommand(command_line);
        }
    }


    // ***********************************************************


    Native::Native(Console::ConsoleCommandServiceInterface *command_service, Foundation::Framework *framework) : Console::ConsoleServiceInterface()
    {
        assert (command_service);
        input_.SetCommandManager(command_service);
        input_.SetFramework(framework);

        thread_ = boost::thread(boost::ref(input_));
    }

    Native::~Native()
    {
    }
}

