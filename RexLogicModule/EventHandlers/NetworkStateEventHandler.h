// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkStateEventHandler_h
#define incl_NetworkStateEventHandler_h

#include "ComponentInterface.h"
#include "CoreTypes.h"

namespace Foundation
{
    class EventDataInterface;
}

namespace RexLogic
{
    class RexLogicModule;

    /// Handles events related to changes in server connectivity in a RexLogic-specific way.
    class NetworkStateEventHandler
    {
    public:
        NetworkStateEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~NetworkStateEventHandler();

        bool HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data);

    private:
        Foundation::Framework *framework_;

        RexLogicModule *rexlogicmodule_;
    };
}

#endif
