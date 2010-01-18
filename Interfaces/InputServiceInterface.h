// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "ServiceInterface.h"
#include "State.h"

namespace Input
{
    /*! input state. The input system is modelled as a Finite State Machine, 
     * and implemented using Qt's State Machine framework.
     *
     * States within the machine are named, and can be queried by name.
     */
    
    /*!
        \ingroup Input_group
    */
    
    //! A service for input events.
    /*! There are two ways to discover what inputs are active: polling and events. 
     *
     * Callbacks can be added to named states by querying the state by name, 
     * and connecting a Qt slot to its onEntry or onExit signals.
     *
     * Events come in duals, and are fired when the named state enters or 
     * exist respectively. Which events fire is stored dynamically and 
     * can be configured by file.
     * 
     * \ingroup Services_group 
     * \ingroup Input_group
     */
    class InputServiceInterface : public Foundation::ServiceInterface
    {
    public:
        InputServiceInterface() {}
        virtual ~InputServiceInterface() {}

        //! Returns the input state requested by name.
        virtual const Foundation::State *GetState (const std::string &name) const = 0;

        //! Fires events on entry or exit respectively.
        virtual void AddEvent (const std::string &state, event_id_t enter, event_id_t exit) const = 0;
    };
}

#endif
