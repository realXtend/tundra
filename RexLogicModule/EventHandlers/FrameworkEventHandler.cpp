// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "EventHandlers/FrameworkEventHandler.h"
#include "EventHandlers/LoginHandler.h"

#include "WorldStream.h"
#include "ScriptServiceInterface.h"
#include "ServiceManager.h"

#include <QMap>

namespace RexLogic
{

bool FrameworkEventHandler::HandleFrameworkEvent(event_id_t event_id, IEventData* data)
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
                // Fix this to work again later (taiga login from command line)
                //CoreUi::LoginContainer *login_ui = rexLogic_->GetLogin();
                //if (login_ui)
                //    login_ui->StartParameterLoginTaiga(parameter);
                //else
                //    rexLogic_->LogInfo("Could not do parameter login due login path now awailable"); // Fix after refactor
            }

            if (command == "-python" || command == "-p")
            {
                if (!parameter.isEmpty())
                {
                    boost::shared_ptr<Foundation::ScriptServiceInterface> pyservice =
                        rexLogic_->GetFramework()->GetServiceManager()->GetService<Foundation::ScriptServiceInterface>(Service::ST_PythonScripting).lock();
                    if (pyservice)
                        pyservice->RunScript(parameter);
                    else
                        RexLogicModule::LogError("Python scripting service not available.");
                }
                else
                    RexLogicModule::LogError("Expected script filename as parameter.");
            }

			//Web realxtend://
			//Expecting start url, avatar url, firstname and lastname. Example realxtend://mydomain.com?http://mydomain.com/avatar.xml?test?test
			if (command.contains("realxtend://"))
			{
				command = command.remove(0, 12);
				if (!command.isEmpty())
				{
					QStringList values = command.split('?');
					if (!values.isEmpty() && values.length() == 4)
					{				
						QString temp = values[2] + " " +values[3];
						rexLogic_->StartLoginOpensim(temp.toStdString().c_str(), "", values[0].toStdString().c_str());

						avatar_address_ = values[1];
					}
				}
			}

        }
    }
	else if (event_id == Foundation::WEB_LOGIN_DATA_RECEIVED)
	{
		Foundation::WebLoginDataEvent *we_event = static_cast<Foundation::WebLoginDataEvent*>(data);
		QString temp = we_event->firstname_ + " " + we_event->lastname_;
		rexLogic_->StartLoginOpensim(temp.toStdString().c_str(), "", we_event->world_address_);

		avatar_address_ = we_event->avatar_address_;
		
	}

    return false;
}

}
