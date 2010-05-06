// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEventHandler_h
#define incl_SceneEventHandler_h

#include "CoreTypes.h"

namespace Foundation
{
    class EventDataInterface;
}

namespace RexLogic
{
    class RexLogicModule;

    class SceneEventHandler
    {
    public:
        explicit SceneEventHandler(RexLogicModule *owner);
        virtual ~SceneEventHandler();

        bool HandleSceneEvent(event_id_t event_id, Foundation::EventDataInterface* data);

    private:
        RexLogicModule *owner_;

        //! handle entity deleted event
        void HandleEntityDeletedEvent(event_id_t entityid);
    };
}

#endif
