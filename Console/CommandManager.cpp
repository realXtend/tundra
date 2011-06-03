// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CommandManager.h"
#include "ConsoleAPI.h"
#include "Native.h"

#include "Framework.h"

#include <boost/tokenizer.hpp>

#include "MemoryLeakCheck.h"

bool nocase_compare(const std::string &lhs, const std::string &rhs)
{
    return boost::algorithm::iequals(lhs, rhs);
}

typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
typedef boost::tokenizer< boost::escaped_list_separator<char> > escape_tokenizer;

CommandManager::CommandManager(ConsoleAPI *console, Foundation::Framework *fw) :
    framework_(fw),
    console_(console),
    nativeinput_(0)
{
    RegisterCommand(CreateConsoleCommand("Help", "Display available commands", ConsoleBind(this, &CommandManager::ConsoleHelp)));
    RegisterCommand(CreateConsoleCommand("Exit", "Exit application", ConsoleBind(this, &CommandManager::ConsoleExit)));
    if (framework_->IsHeadless())
        nativeinput_ = new NativeConsole(this);
}

CommandManager::~CommandManager()
{
    SAFE_DELETE(nativeinput_);
}

void CommandManager::Update()
{
    while (commandlines_.empty() == false)
    {
        std::string command_line;
        {
            MutexLock lock(commandlines_mutex_);
            command_line = commandlines_.front();
            commandlines_.pop();
        }

        ExecuteCommand(command_line);
    }
}

void CommandManager::QueueCommand(const std::string &commandline)
{
    MutexLock lock(commandlines_mutex_);
    commandlines_.push(commandline);
}

void CommandManager::RegisterCommand(const ConsoleCommandStruct &command)
{
    RecursiveMutexLock lock(commands_mutex_);

    if (commands_.find(command.name_) != commands_.end())
    {
        RootLogWarning("Command " + command.name_ + " already registered.");
        return;
    }

    std::string name = command.name_;
    boost::to_lower(name);
    commands_[name] = command;
}

void CommandManager::UnregisterCommand(const std::string &name)
{
    RecursiveMutexLock lock(commands_mutex_);

    std::string name_low = name;
    boost::to_lower(name_low);
    CommandMap::iterator it = commands_.find(name_low);
    if (it == commands_.end())
    {
        RootLogWarning("Trying to unregister command " + name + ", but it has not been registered.");
        return;
    }
    commands_.erase(it);
}

ConsoleCommandResult CommandManager::ExecuteCommand(const std::string &commandline)
{
    if (commandline.empty())
        return ConsoleResultFailure();

    std::string cl = commandline;

    // separate command and parameters
    boost::char_separator<char> sep("([{]})");
    tokenizer commandline_tok(cl, sep);
    
    std::string command;
    std::string param_line;
    StringVector params;

    tokenizer::iterator it = commandline_tok.begin();
    if (it == commandline_tok.end())
    {
        console_->Print_("Failed to parse malformed command line: " + commandline);
        ConsoleCommandResult result = { false, "", false };
        return result;
    }

    command = *it;
    boost::trim(command);

    ++it;
    if (it != commandline_tok.end())
    {
        param_line = *it;
        // Separate parameters and push to vector
        boost::escaped_list_separator<char> param_sep;
        escape_tokenizer param_tok(param_line);
        try
        {
            for (escape_tokenizer::iterator it = param_tok.begin();it != param_tok.end() ; ++it)
            {
                std::string param = *it;
                boost::trim(param);
                params.push_back(param);
            }
        }
        catch (boost::escaped_list_error &/*e*/)
        {
            console_->Print_("Invalid use of escaping.");
            ConsoleCommandResult result = { false, "", false };
            return result;
        }
    }

    /* an alternative version that allows using () inside the command arguments, useful for pyexec & jsexec
      \todo fix the impl above to support parentheses too
    std::string command;
    std::string param_line;
    StringVector params;

    size_t offset = cl.find_first_of("(");
    command = cl.substr(0, offset);
    param_line = cl.substr(offset + 1, cl.length() - 2); //find last ) instead?
    params.push_back(param_line);*/

    return ExecuteCommand(command, params);
}

ConsoleCommandResult CommandManager::ExecuteCommandAlways(const std::string &name, const StringVector &params, bool always)
{
    std::string low_name = name;
    boost::to_lower(low_name);
    
    ConsoleCallbackPtr callback;
    bool delayed = false;
    UNREFERENCED_PARAM(delayed);
    {
        RecursiveMutexLock lock(commands_mutex_);
        CommandMap::const_iterator iter = commands_.find(low_name);
        if (iter == commands_.end())
        {
            console_->Print_("Command: " + name + " not found. Type 'help' for list of available commands.");
            ConsoleCommandResult result = { false, "", false };
            return result;
        }
        callback = iter->second.callback_;
        if (!always && iter->second.delayed_)
        {
            delayed_commands_[iter->first] = params;
            return ConsoleResultDelayed();
        }
    }

    // If no callback, we have Qt slot console command
    if (!callback)
    {
        QStringList qparams;
        for(size_t i = 0; i < params.size(); ++i)
            qparams << params[i].c_str();
        emit CommandInvoked(name.c_str(), qparams);
        ConsoleCommandResult result = { true, "", false };
        return result;
    }

    ConsoleCommandResult result = (*callback)(params);
    if (result.why_.empty() == false)
    {
        if (result.success_ == false)
            console_->Print_("Error: " + result.why_);
        else
            console_->Print_(result.why_);
    }
    return result;
}

ConsoleCommandResult CommandManager::ConsoleHelp(const StringVector &params)
{
    if (params.empty())
    {
        console_->Print_("Available commands with descriptions:");
    }
    bool success = false;

    {   
        RecursiveMutexLock lock(commands_mutex_);
        CommandMap::const_iterator it = commands_.begin();
        for ( ; it != commands_.end() ; ++it)
        {
            if (params.empty() || std::find_if(params.begin(), params.end(), boost::bind( &nocase_compare, _1, it->second.name_ )) != params.end())
            {
                console_->Print_(it->second.name_ + " - " + it->second.description_);
                success = true;
            }
        }
    }

    if (!success)
    {
        assert (params.empty() == false);
        return ConsoleResultFailure("Unknown command " + params[0]);
    }

    if (params.empty())
    {
        console_->Print_("");
        console_->Print_("For help with specific command, type help(command).");
    }

    return ConsoleResultSuccess();
}

ConsoleCommandResult CommandManager::ConsoleExit(const StringVector &params)
{
    console_->Print_("Exiting");
    framework_->Exit();

    return ConsoleResultSuccess();
}

