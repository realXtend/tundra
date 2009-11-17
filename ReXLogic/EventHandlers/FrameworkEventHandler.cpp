// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/FrameworkEventHandler.h"
#include "WorldStream.h"
#include "Login/LoginHandler.h"

#include <QMap>

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
                QMap<QString, QString> map;
			    map["WorldAddress"] = QString(options["server"].as<std::string>().c_str());
			    map["Username"] = QString(options["user"].as<std::string>().c_str());
			    map["Password"] = QString(options["passwd"].as<std::string>().c_str());

                //std::string auth_login; // Whats this for??
                if (options.count("auth_server") &&
                    options.count("auth_login"))
                {
                    map["AuthenticationAddress"] = QString(options["auth_server"].as<std::string>().c_str());
                    //auth_login = options["auth_login"].as<std::string>();
                }

                OpenSimLoginHandler *loginHandler = new OpenSimLoginHandler(framework_, rexLogic_);
                if (options.count("auth_server"))
                    loginHandler->ProcessRealXtendLogin(map);
                else
                    loginHandler->ProcessOpenSimLogin(map);

                //bool succesful = connection_->ConnectToServer(user, passwd, server, auth_server, auth_login);
            }
        }
        else if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            Foundation::NetworkingRegisteredEvent *event_data = dynamic_cast<Foundation::NetworkingRegisteredEvent *>(data);
            if (event_data)
                rexLogic_->SubscribeToNetworkEvents(event_data->currentProtocolModule);
        }

        return false;
    }
}
