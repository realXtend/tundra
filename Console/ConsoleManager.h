// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_ConsoleManager_h
#define incl_Console_ConsoleManager_h

#include "CommandManager.h"
#include "LogListenerInterface.h"

class Framework;

class CommandManager;
class ConsoleChannel;
class LogListener;

typedef boost::shared_ptr<LogListener> LogListenerPtr;
typedef boost::shared_ptr<ConsoleChannel> PocoLogChannelPtr;

//! Generic debug console manager, directs input and output to available consoles.
/*!
    See \ref DebugConsole "Using the debug console".
*/
class ConsoleManager //:  public ConsoleServiceInterface
{
public:
    explicit ConsoleManager(Framework *fw);

    //! destructor
    virtual ~ConsoleManager();

    __inline virtual void Update(f64 frametime);

    //! Print text in parameter
    __inline virtual void Print(const std::string &text);

    //! Execute command in parameter
    virtual void ExecuteCommand(const std::string &command);

    //! Toggle console on/off
    virtual void ToggleConsole() {}

    //! Sets Ui initialized/uninitialized
    virtual void SetUiInitialized(bool initialized);

    //! Returns false if UI is not initialized, true otherwise
    virtual bool IsUiInitialized() const { return ui_initialized_; }

    //! Returns command manager
    CommandManager *GetCommandManager() const { return command_manager_; }

    //! Removes the Console from the list of Ogre log listeners.
    void UnsubscribeLogListener();

private:
    Q_DISABLE_COPY(ConsoleManager);

    Framework *framework_;

    //! command manager
    CommandManager *command_manager_;

    //! Custom logger to get logmessages from Pogo
    PocoLogChannelPtr console_channel_;

    //! Listener to get logs from renderer 
    LogListenerPtr log_listener_;

    //! This is a buffer for messages generated before actual console UI
    std::vector<std::string> early_messages_;

    //!indicates whether the UI is initialized
    bool ui_initialized_;
};

//! Used to listen log messages from renderer
class LogListener
{
    LogListener();

public:
    explicit LogListener(ConsoleManager *console) : mngr_(console) {}
    virtual ~LogListener() {}

    /// A very generic log message passing, only the message itself.
    /// Does not handle log levels or anything extra.
    virtual void LogMessage(const std::string &message){ if(mngr_) mngr_->Print(message); }
    ConsoleManager* mngr_;
};

//! Class to get messages from poco logger
class ConsoleChannel//: public Poco::Channel
{
public:
    explicit ConsoleChannel(ConsoleManager* mngr){ mngr_ = mngr; }
//    void log(const Poco::Message & msg){ if (mngr_) mngr_->Print(msg.getText()); }
private:
    ConsoleManager* mngr_;
};

#endif

