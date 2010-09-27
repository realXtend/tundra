// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEventHandler_h
#define incl_SceneEventHandler_h

#include "CoreTypes.h"
#include <QList>

class IEventData;

namespace RexLogic
{
    class RexLogicModule;

    class SceneEventHandler
    {
    public:
        explicit SceneEventHandler(RexLogicModule *owner);
        virtual ~SceneEventHandler();
        bool HandleSceneEvent(event_id_t event_id, IEventData* data);

    private:
        RexLogicModule *owner_;
        QList<event_id_t> hovered_entitys_;
    };
}

#endif
