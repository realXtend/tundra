// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkStateEventHandler.h"
#include "OpenSimProtocolModule.h"
#include "RexLogicModule.h"

namespace RexLogic
{

NetworkStateEventHandler::NetworkStateEventHandler(
    Foundation::Framework *framework,
    RexLogicModule *rexlogicmodule) :
    framework_(framework),
    rexlogicmodule_(rexlogicmodule)
{
}

// virtual
NetworkStateEventHandler::~NetworkStateEventHandler()
{
}

bool NetworkStateEventHandler::HandleNetworkStateEvent(
    Core::event_id_t event_id,
    Foundation::EventDataInterface* data)
{
    switch(event_id)
    {
    case OpenSimProtocol::Events::EVENT_SERVER_CONNECTED:
    {
        // The client has connected to the server. Create a new scene for that.
        rexlogicmodule_->CreateNewActiveScene("World");

        // Initialize the inventory tree view.
        rexlogicmodule_->InitInventoryView();
        break;
    }
    case OpenSimProtocol::Events::EVENT_SERVER_DISCONNECTED:
    {
        // Might be user quitting or server dropping connection.
        // This event occurs when OpenSimProtocolModule has already closed connection. 
        // Make sure the rexlogic also thinks connection is closed.
        if (rexlogicmodule_->GetServerConnection()->IsConnected())
            rexlogicmodule_->GetServerConnection()->ForceServerDisconnect();

        if (framework_->HasScene("World"))
            rexlogicmodule_->DeleteScene("World");

        rexlogicmodule_->ResetInventoryView();
        break;
    }
    default:
        break;
    }

    return false;
}

}
