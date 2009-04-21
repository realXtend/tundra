// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputEventHandler_h
#define incl_InputEventHandler_h

#include "Foundation.h"

namespace RexLogic
{
    class InputStateInterface;
    class RexLogicModule;

    class InputEventHandler
    {
        typedef boost::weak_ptr<InputStateInterface> InputStatePtr;
    public:
        InputEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~InputEventHandler();
        
        bool HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Set input state. Input will be handled according to the state.
        /*!
            \param state shared or weak ptr to the state object
        */
        void SetState(const InputStatePtr &state) { state_ = state; }

        void Update(Core::f64 frametime);

    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;

        //! current state for handling input
        InputStatePtr state_;
    };
}

#endif
