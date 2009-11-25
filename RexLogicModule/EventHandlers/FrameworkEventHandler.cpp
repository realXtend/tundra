// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/FrameworkEventHandler.h"
#include "WorldStream.h"
#include "Login/LoginHandler.h"
#include "Login/LoginUI.h"

#include <QMap>

namespace RexLogic
{

    bool FrameworkEventHandler::HandleFrameworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Foundation::PROGRAM_OPTIONS)
        {
			QMap<int, QString> map;
			QString command, parameter;
            Foundation::ProgramOptionsEvent *po_event = static_cast<Foundation::ProgramOptionsEvent*>(data);

			for( int count = 0; count < po_event->argc; count++ )
				map[count] = QString(po_event->argv[count]);

			command = map[1];
			parameter = map[2];

			if (!command.isEmpty())
            {
                if (command == "-loginuri" && !parameter.isEmpty() && parameter.startsWith("http://", Qt::CaseInsensitive))
                {
    				rexLogic_->GetLogin()->StartParameterLoginTaiga(parameter);
                }

    			if (command == "-python" || command == "-p")                   
                {
                    if (!parameter.isEmpty())
                    {
                        boost::shared_ptr<Foundation::ScriptServiceInterface> pyservice = framework_->GetServiceManager()->GetService<Foundation::ScriptServiceInterface>(Foundation::Service::ST_Scripting).lock();
                        if (pyservice)
                        {
                            pyservice->RunScript(parameter.toStdString());
                        }
                        //\todo else report error: py scripting service not available
                    }
                    //\todo else report error: expected script filename as param
                }
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
