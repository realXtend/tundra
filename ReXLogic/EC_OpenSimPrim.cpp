// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimPrim.h"
#include "NetInMessage.h"

namespace RexLogic
{
    EC_OpenSimPrim::EC_OpenSimPrim(Foundation::ModuleInterface* module)
    {
        RegionHandle = 0;
        LocalId = 0;
        
        ObjectName = "";
        Description = "";
    }

    EC_OpenSimPrim::~EC_OpenSimPrim()
    {
    }
    
    void EC_OpenSimPrim::HandleObjectUpdate(Foundation::EventDataInterface* data)
    {
        // RexNetworkEventData *rexdata = static_cast<RexLogic::RexNetworkEventData *>(data);
        NetInMessage *msg = NULL; // // todo tucofixme, rexdata->Message;    
    
        // TODO: tucofixme set values based on data
        RegionHandle = 0;
        LocalId = 0;
        // TODO: tucofixmeFullId = ""; 
        // TODO: tucofixmeOwnerId = "";
        // TODO: tucofixmeParentId = "";
        
        ObjectName = "";
        Description = "";
    }
}
