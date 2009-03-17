// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <boost/tokenizer.hpp>
#include "Native.h"
#include "ConsoleModule.h"


namespace Console
{
    typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
    typedef boost::tokenizer< boost::escaped_list_separator<char> > escape_tokenizer;

    Native::Native() : Foundation::Console::ConsoleServiceInterface()
    {
    }

    Native::~Native()
    {
    }
    void Native::RegisterCommand(const Foundation::Console::Command &command)
    {
        if (commands_.find(command.name_) != commands_.end())
        {
            ConsoleModule::LogError("Command " + command.name_ + " already registered.");
            return;
        }
    
        std::string name = command.name_;
        boost::to_lower(name);
        commands_[name] = command;
    }

    void Native::ExecuteCommand(const std::string &commandline)
    {
        std::string cl = commandline;

        // separate command and parameters
        boost::char_separator<char> sep("([{)]}");
        tokenizer commandline_tok(cl, sep);

        
        std::string command;
        std::string param_line;
        Core::StringVector params;

        tokenizer::iterator it = commandline_tok.begin();
        if (it == commandline_tok.end())
        {
            ConsoleModule::LogInfo("Failed to parse malformed command line: " + commandline);
            return;
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

            for (escape_tokenizer::iterator it = param_tok.begin() ;
                 it != param_tok.end() ; 
                 ++it)
            {
                std::string param = *it;
                boost::trim(param);
                params.push_back(param);
            }
        }

        ExecuteCommand(command, params);
    }

    void Native::ExecuteCommand(const std::string &name, const Core::StringVector &params)
    {
        std::string low_name = name;
        boost::to_lower(low_name);

        CommandMap::const_iterator iter = commands_.find(low_name);
        if (iter == commands_.end())
        {
            ConsoleModule::LogInfo("Command: " + name + " not found.");
            return;
        }

        (*iter->second.callback_)(params);
    }
}

