// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEventHandler_h
#define incl_SceneEventHandler_h

#include "ComponentInterface.h"
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
        SceneEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~SceneEventHandler();
        
        bool HandleSceneEvent(event_id_t event_id, Foundation::EventDataInterface* data);
        
    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
        
        //! handle entity deleted event
        void HandleEntityDeletedEvent(event_id_t entityid);
    };
}

#endif
