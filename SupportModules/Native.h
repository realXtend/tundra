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
    class ConsoleCommandServiceInterface;
    
    class NativeInput
    {
    public:
        /// constructor
        NativeInput() : command_service_(0) { }
        /// destructor
        ~NativeInput() {}
        /// (thread) entry point
        void operator()();

        void SetCommandManager(Console::ConsoleCommandServiceInterface *command_service) { command_service_ = command_service; assert (command_service_); }
        void SetFramework(Foundation::Framework *framework) { framework_ = framework; assert (framework_); }

    private:
        NativeInput(const NativeInput &other);

        Console::ConsoleCommandServiceInterface *command_service_;
        Foundation::Framework *framework_;
    };

    /// Native debug input console
    class Native
    {
        Native();
    public:
        /// constructor
        Native(Console::ConsoleCommandServiceInterface *command_service, Foundation::Framework *framework);
        /// destructor
        virtual ~Native();

    private:

        /// input thread
        Thread thread_;

        /// Handles input from native console
        NativeInput input_;
    };
}

#endif

