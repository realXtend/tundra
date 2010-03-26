// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "ServiceInterface.h"
#include "State.h"
#include "KeyBindings.h"

#include <QString>
#include <QKeySequence>

namespace Foundation
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

    class KeyBindings;

    class InputServiceInterface : public Foundation::ServiceInterface
    {
    public:
        InputServiceInterface() {}
        virtual ~InputServiceInterface() {}

        //! Returns the input state requested by name.
        virtual Foundation::State *GetState (QString name) = 0;

        //! Return the currently used key bindings
        virtual Foundation::KeyBindings *GetBindings() = 0;

        //! Set in param bindings to be used
        virtual void SetBindings(Foundation::KeyBindings *bindings) = 0;

        //! Restore the default binginds
        virtual void RestoreDefaultBindings() = 0;

        //! Fires events on entry or exit respectively.
        virtual void AddKeyEvent (QString group, QString key_sequence, event_id_t enter, event_id_t exit) = 0;
    };
}

#endif
