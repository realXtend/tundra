#ifndef incl_RexLogicModule_ComponentResourceHandler_h
#define incl_RexLogicModule_ComponentResourceHandler_h

#include "ModuleInterface.h"
#include "Framework.h"
#include "CoreTypes.h"

namespace RexLogic
{
    //! @todo quick and dirty way to handle resource ready event to components. This should be removed when propper event handling is possible with entity components.
    class ComponentResourceHandler
    {
        public:
            ComponentResourceHandler(Foundation::ModuleInterface *module);
            void HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        private:
            Foundation::Framework *framework_;
    };
}
#endif