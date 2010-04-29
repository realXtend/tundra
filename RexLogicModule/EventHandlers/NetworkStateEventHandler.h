// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkStateEventHandler_h
#define incl_NetworkStateEventHandler_h

#include "CoreTypes.h"
#include "RexUUID.h"

#include <QList>

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
        /// Constructor
        /// @param owner Owner module.
        explicit NetworkStateEventHandler(RexLogicModule *owner);

        /// Destructor.
        virtual ~NetworkStateEventHandler();

        /// Handles
        /// @param event_id
        /// @param data
        bool HandleNetworkStateEvent(event_id_t event_id, Foundation::EventDataInterface* data);

    private:
        /// Owner module.
        RexLogicModule *owner_;

        /// Trying to avoid double notifications, somehow for every avatar we go twice to 
        /// HandleOSNE_ObjectUpdate() when there are already avatars inworld
        QList<RexUUID> sent_avatar_notifications_;
    };
}

#endif
