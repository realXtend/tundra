// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarkEventHandler_h
#define incl_AvatarkEventHandler_h

#include "CoreTypes.h"
#include "RexUUID.h"

namespace RexLogic
{
    class RexLogicModule;

    //! Handles avatar events
    class AvatarEventHandler
    {
    public:
        AvatarEventHandler(RexLogicModule *owner) :
            owner_(owner) {}
        ~AvatarEventHandler() {}

        //! Handle avatar events
        bool HandleAvatarEvent(event_id_t event_id, IEventData* data);

        //! Send avatar events
        void SendRegisterEvent(const RexUUID &fullid, entity_id_t entityid);
        void SendUnregisterEvent(const RexUUID &fullid);

    private:
        //! Owner module
        RexLogicModule *owner_;
    };
}

#endif
