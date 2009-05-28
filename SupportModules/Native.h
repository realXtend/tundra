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
    class NativeInput
    {
    public:
        //! constructor
        NativeInput() : command_service_(NULL) { }
        //! destructor
        ~NativeInput() {}
        //! (thread) entry point
        void operator()();

        void SetCommandManager(Console::ConsoleCommandServiceInterface *command_service) { command_service_ = command_service; assert (command_service_); }
        void SetFramework(Foundation::Framework *framework) { framework_ = framework; assert (framework_); }

    private:
        NativeInput(const NativeInput &other);

        Console::ConsoleCommandServiceInterface *command_service_;
        Foundation::Framework *framework_;
    };

    //! Native debug console
    class Native : public Console::ConsoleServiceInterface
    {
        Native();
        Native(const Native &other);
    public:
        //! constructor
        Native(Console::ConsoleCommandServiceInterface *command_service, Foundation::Framework *framework);
        //! destructor
        virtual ~Native();

        __inline virtual void Print(const std::string &text)
        {
            std::cout << text << std::endl;
        }

        virtual void Scroll(int rel) {}
        virtual void SetVisible(bool visible) {}
        virtual bool IsVisible() const { return true; }
        virtual bool IsActive() const {return true; }
        virtual bool HandleKeyDown(int code, Core::uint text) { return false; }
        virtual bool HandleKeyUp(int code, Core::uint text) { return false; }
    private:

        //! input thread
        Core::Thread thread_;

        //! Handles input from native console
        NativeInput input_;
    };
}

#endif

