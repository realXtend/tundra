// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_EC_Controllable_h
#define incl_RexLogic_EC_Controllable_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"

namespace RexLogic
{
    //! Controllable entity component.
    /*! Any entity with this component may be controlled by the user.
        The entity can be controlled via 'actions'. Each different type
        of controllable has their own list of actions, so they work in a
        generic fashion. 
        
        Many actions will probably be common to most or all controllables,
        but actions can also be specific to a type of controllable.
        F.ex. a tank could have a unique action 'rotate turret'.
    */
    class EC_Controllable : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_Controllable);
    public:
        virtual ~EC_Controllable() {}

        //! Add a new action for this controllable
        void AddAction(int action)
        { 
            assert(actions_.find(action) == actions_.end() && "Action already added to this controller.");
            actions_.insert(action);
        }

        //! Sets current action for this controllable. Event for the action is send in a delayed fashion.
        void SetCurrentAction(int action)
        { 
            assert (action != 0 && "Action ID 0 reserved for internal use.");
            assert(actions_.find(action) != actions_.end() && "Action not supported by this controller.");

            current_action_ = action;

            dirty_ = true;
        }

        //! Set unique name for this controllable
        void SetName(const std::string &name) { name_ = name; }

    private:
        typedef std::set<int> ActionSet;

        //! list of actions supported by this controller
        ActionSet actions_;

        //! Currently active action for this controllable
        int current_action_;

        //! If true, an action is pending and needs to be sent
        bool dirty_;

        //! Name of this controllable
        std::string name_;

        EC_Controllable(Foundation::ModuleInterface* module) : current_action_(0), dirty_(false) {}
    };
}

#endif
