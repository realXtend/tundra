// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleConsoleManager_h
#define incl_ConsoleConsoleManager_h

#include "ConsoleServiceInterface.h"
#include "Native.h"
#include "CommandManager.h"

namespace Console
{
    //! Native debug console
    class ConsoleManager : public Console::ConsoleServiceInterface
    {
        friend class ConsoleModule;
    private:
        ConsoleManager();
        ConsoleManager(const ConsoleManager &other);

        //! constructor that takes a parent module
        ConsoleManager(Foundation::ModuleInterface *parent)
        {
            parent_ = parent;
            command_manager_ = CommandManagerPtr(new CommandManager(parent_, this));
            native_ = ConsolePtr(new Native(command_manager_.get()));
        }

    public:
        //! destructor
        virtual ~ConsoleManager() {};

        //! Prints text to the console (all consoles)
        __inline virtual void Print(const std::string &text)
        {
            native_->Print(text);
        }

        //! Returns command manager
        CommandManagerPtr GetCommandManager() const {return command_manager_; }

    private:
        //! native debug console
        ConsolePtr native_;

        //! command manager
        CommandManagerPtr command_manager_;

        //! parent module
        Foundation::ModuleInterface *parent_;
    };
}

#endif

