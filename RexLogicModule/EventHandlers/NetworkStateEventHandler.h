/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   NetworkStateEventHandler.h
 *  @brief  Handles events related to changes in server connectivity in a RexLogic-specific way.
 */

#ifndef incl_NetworkStateEventHandler_h
#define incl_NetworkStateEventHandler_h

#include "CoreTypes.h"

class IEventData;

namespace RexLogic
{
    class RexLogicModule;

    class NetworkStateEventHandler
    {
    public:
        /// Constructor
        /// @param owner Owner module.
        explicit NetworkStateEventHandler(RexLogicModule *owner);

        /// Destructor.
        virtual ~NetworkStateEventHandler();

        /// Handles
        /// @param event_id Event ID.
        /// @param data Event data.
        bool HandleNetworkStateEvent(event_id_t event_id, IEventData* data);

    private:
        /// Owner module.
        RexLogicModule *owner_;
    };
}

#endif
