// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/NetworkStateEventHandler.h"
#include "RexLogicModule.h"
#include "NetworkEvents.h"
#include "Framework.h"
#include "EventManager.h"
#include "WorldStream.h"

namespace RexLogic
{
    NetworkStateEventHandler::NetworkStateEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
        : framework_(framework), rexlogicmodule_(rexlogicmodule)
    {
    }

    NetworkStateEventHandler::~NetworkStateEventHandler()
    {
    }
    
    bool NetworkStateEventHandler::HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        switch(event_id)
        {
            case ProtocolUtilities::Events::EVENT_SERVER_CONNECTED:
            {   
                // The client has connected to the server. Create a new scene for that.
                rexlogicmodule_->CreateNewActiveScene("World");
                // Send WorldStream as internal event
                event_category_id_t framework_category_id = framework_->GetEventManager()->QueryEventCategory("Framework");
                ProtocolUtilities::WorldStreamReadyEvent event_data(rexlogicmodule_->GetServerConnection());
                framework_->GetEventManager()->SendEvent(framework_category_id, Foundation::WORLD_STREAM_READY, &event_data);
                break;
            }
            case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
            {
                // Might be user quitting or server dropping connection.
                // This event occurs when OpenSimProtocolModule has already closed connection. 
                // Make sure the rexlogic also thinks connection is closed.
                if (rexlogicmodule_->GetServerConnection()->IsConnected())
                    rexlogicmodule_->GetServerConnection()->ForceServerDisconnect();
                if (framework_->HasScene("World"))
                    rexlogicmodule_->DeleteScene("World");
                break;
            }
            default:
                break;
        }
        
        return false;
    }
}
