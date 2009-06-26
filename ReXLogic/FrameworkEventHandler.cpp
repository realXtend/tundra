// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "FrameworkEventHandler.h"
#include "RexServerConnection.h"

namespace RexLogic
{
    bool FrameworkEventHandler::HandleFrameworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Foundation::PROGRAM_OPTIONS)
        {
            Foundation::ProgramOptionsEvent *po_event = static_cast<Foundation::ProgramOptionsEvent*>(data);
            
            const boost::program_options::variables_map &options = po_event->options;

            if (options.count("user")   &&
                options.count("passwd") &&
                options.count("server") &&
                options.count("login"))
            {
                const std::string &user = options["user"].as<std::string>();
                const std::string &passwd = options["passwd"].as<std::string>();
                const std::string &server = options["server"].as<std::string>();

                std::string auth_server;
                std::string auth_login;
                if (options.count("auth_server") &&
                    options.count("auth_login"))
                {
                    auth_server = options["auth_server"].as<std::string>();
                    auth_login = options["auth_login"].as<std::string>();
                }

                bool succesful = connection_->ConnectToServer(user, passwd, server, auth_server, auth_login);
            }
        }

        return false;
    }
}
