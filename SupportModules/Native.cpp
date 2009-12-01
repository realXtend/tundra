// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Native.h"
#include "ConsoleModule.h"
    
#include "MemoryLeakCheck.h"

namespace Console
{
    void NativeInput::operator()()
    {
        #ifndef WINDOWS_APP    
        assert (command_service_);

        while (true)
        {
            std::string command_line;
            std::getline(std::cin, command_line);

            boost::this_thread::interruption_point();

            if (std::cin.fail())
            {
                if (framework_)
                    framework_->Exit();
                break;
            }

            command_service_->QueueCommand(command_line);
        }
        #endif        
    }


    // ***********************************************************


    Native::Native(Console::ConsoleCommandServiceInterface *command_service, Foundation::Framework *framework) : Console::ConsoleServiceInterface()
    {
        assert (command_service);
        input_.SetCommandManager(command_service);
        input_.SetFramework(framework);

        // Disabled for now due to a bug in the above thread code. std::getline blocks!
        // thread_ = boost::thread(boost::ref(input_));
    }

    Native::~Native()
    {
        thread_.interrupt();
        thread_.join();
    }
}

