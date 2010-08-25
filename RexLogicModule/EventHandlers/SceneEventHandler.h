// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEventHandler_h
#define incl_SceneEventHandler_h

#include "CoreTypes.h"
#include <QList>

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

        //! Clears the hovered entitys, for touchable ec:s calls HoverOut()
        void ClearHovers(event_id_t entity_id);

    private:
        RexLogicModule *owner_;

        //! handle entity deleted event
        void HandleEntityDeletedEvent(event_id_t entityid);

        QList<event_id_t> hovered_entitys_;
    };
}

#endif
