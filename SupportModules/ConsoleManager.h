// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleConsoleManager_h
#define incl_ConsoleConsoleManager_h

#include <Poco/Channel.h>
#include <Poco/Message.h>

#include "ConsoleServiceInterface.h"
#include "CommandManager.h"
#include "LogListenerInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Console
{
    class ConsoleChannel;
    class LogListener;

    typedef boost::shared_ptr<LogListener> LogListenerPtr;
    typedef boost::shared_ptr<ConsoleChannel> PocoLogChannelPtr;

    //! Generic debug console manager, directs input and output to available consoles.
    /*!
        See \ref DebugConsole "Using the debug console".
    */
    class ConsoleManager :  public Console::ConsoleServiceInterface
    {
        friend class ConsoleModule;
    private:
        ConsoleManager();
        ConsoleManager(const ConsoleManager &other);

        //! constructor that takes a parent module
        ConsoleManager(Foundation::ModuleInterface *parent);

    public:
        //! destructor
        virtual ~ConsoleManager();

        __inline virtual void Update(f64 frametime);

        //! Print text in parameter
        __inline virtual void Print(const std::string &text);

        //! Execute command in parameter
        virtual void ExecuteCommand(const std::string &command);

        //! Toggle console on/off
        virtual void ToggleConsole();

        //! Sets Ui initialized/uninitialized
        virtual void SetUiInitialized(bool initialized);

        //! Returns false if UI is not initialized, true otherwise
        virtual bool IsUiInitialized(){return ui_initialized_;}

        //! Returns command manager
        CommandManagerPtr GetCommandManager() const {return command_manager_; }

    private:
        //Console event category
        event_category_id_t console_category_id_;

        //! command manager
        CommandManagerPtr command_manager_;

        //! parent module
        Foundation::ModuleInterface *parent_;
        
        //! framework
        Foundation::Framework* framework_;

        //! Custom logger to get logmessages from Pogo
        PocoLogChannelPtr console_channel_;

        //! Listener to get logs from renderer 
        LogListenerPtr log_listener_;

        //! This is a buffer for messages generated before actual console UI
        std::vector<std::string> early_messages_;

        //!indicates whether the UI is initialized
        bool ui_initialized_;
    };

    //! loglistener is used to listen log messages from renderer
    class LogListener : public Foundation::LogListenerInterface
    {
        LogListener();

    public:
        LogListener(ConsoleManager *console) : Foundation::LogListenerInterface(), mngr_(console) {}
        virtual ~LogListener() {}

        virtual void LogMessage(const std::string &message){ if(mngr_) mngr_->Print(message); }
        ConsoleManager* mngr_;
    };

    //! Class to get messages from poco logger
    class ConsoleChannel: public Poco::Channel
    {
    public:
        ConsoleChannel(ConsoleManager* mngr){ mngr_ = mngr; }
        void log(const Poco::Message & msg){ if (mngr_) mngr_->Print(msg.getText()); }
    private:
        ConsoleManager* mngr_;
    };
}

#endif

