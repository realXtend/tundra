// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleConsoleManager_h
#define incl_ConsoleConsoleManager_h

#include "ConsoleServiceInterface.h"
#include "Native.h"
#include "OgreOverlay.h"
#include "CommandManager.h"

namespace Console
{
    //! Generic debug console manager, directs input and output to available consoles.
    /*!
        See \ref DebugConsole "Using the debug console".
    */
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
            ogre_ = ConsolePtr(new OgreOverlay(parent));
        }

    public:
        //! destructor
        virtual ~ConsoleManager() {};

        //! If console manager gets created in preinit, or at time when not all needed services are present,
        //! this functions can be used for delayed initialization / creation.
        void CreateDelayed() { checked_static_cast<OgreOverlay*>(ogre_.get())->Create(); }

        __inline virtual void Update(Core::f64 frametime)
        {
            command_manager_->Update();
            if (ogre_->IsVisible())
            {
                ogre_->Update(frametime);
            }
        }

        __inline virtual void Print(const std::string &text)
        {
            native_->Print(text);
            ogre_->Print(text);
        }

        virtual void Scroll(int rel)
        {
            native_->Scroll(rel);
            ogre_->Scroll(rel);
        }

        virtual void SetVisible(bool visible)
        {
            ogre_->SetVisible(visible);
        }

        virtual bool IsVisible() const
        {
            return ogre_->IsVisible();
        }

        virtual bool IsActive() const
        {
            return ogre_->IsActive();
        }

        virtual bool HandleKeyDown(int code, Core::uint text)
        {
            return ogre_->HandleKeyDown(code, text);
        }

        virtual bool HandleKeyUp(int code, Core::uint text)
        {
            return ogre_->HandleKeyUp(code, text);
        }

        //! Returns command manager
        CommandManagerPtr GetCommandManager() const {return command_manager_; }

        //! Returns Ogre console
        ConsolePtr GetOgre() const { return ogre_; }

        //! Returns native console
        ConsolePtr GetNative() const { return native_; }

    private:
        //! native debug console
        ConsolePtr native_;

        //! Ogre debug console
        ConsolePtr ogre_;

        //! command manager
        CommandManagerPtr command_manager_;

        //! parent module
        Foundation::ModuleInterface *parent_;
    };
}

#endif

