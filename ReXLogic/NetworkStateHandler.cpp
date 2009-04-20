// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkStateEventHandler.h"
#include "OpenSimProtocolModule.h"
#include "RexLogicModule.h"

namespace RexLogic
{
    NetworkStateEventHandler::NetworkStateEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    : framework_(framework), rexlogicmodule_(rexlogicmodule)
    {
    }

    NetworkStateEventHandler::~NetworkStateEventHandler()
    {
    }
    
    bool NetworkStateEventHandler::HandleNetworkStateEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        switch(event_id)
        {
            case OpenSimProtocol::Events::EVENT_SERVER_CONNECTED:
                {   
                    // The client is connected to the server. Create the "World" scene.
                    Foundation::SceneManagerServiceInterface *sceneManager = 
                        framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
                    if (!sceneManager->HasScene("World"))
                        sceneManager->CreateScene("World");

                    // Also create a default terrain to RexLogic. This is done here dynamically instead of fixed in RexLogic,
                    // since we might have 0-N terrains later on, depending on where we actually connect to. Now of course
                    // we just create one default terrain.
                    rexlogicmodule_->CreateTerrain();

                    break;
                }
            default:
                break;
        }
        
        return false;
    }
}
