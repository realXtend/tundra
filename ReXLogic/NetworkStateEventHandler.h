// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkStateEventHandler_h
#define incl_NetworkStateEventHandler_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    class RexLogicModule;

    class NetworkStateEventHandler
    {
    public:
        NetworkStateEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~NetworkStateEventHandler();
        
        bool HandleNetworkStateEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
    };
}

#endif
