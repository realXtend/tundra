// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_EC_Controllable_h
#define incl_RexLogic_EC_Controllable_h

#include "IComponent.h"
#include "RexTypes.h"
#include "InputEvents.h"
#include "RexLogicModuleApi.h"
#include "Declare_EC.h"
#include "IModule.h"

namespace RexLogic
{
    //! Controllable entity component.
    /*! Any entity with this component may be controlled by the user.
        The entity can be controlled via 'actions'. Each different type
        of controllable has their own list of actions, so they work in a
        generic fashion. 

        Controllables have a type. For each different type, a controller
        class exists which handles the actual movement of the entity based
        on the actions a controllable may perform.
        
        Many actions will probably be common to most or all controllables,
        but actions can also be specific to a type of controllable.
        F.ex. a tank could have a unique action 'rotate turret'.

        For an example of a custom controller for an entity, see
        AvatarControllable.
        
    */

	/**

<table class="header">
<tr>
<td>
<h2>Controllable</h2>
		Does not emit any actions.
		Any entity with this component may be controlled by the user.
        The entity can be controlled via 'actions'. Each different type
        of controllable has their own list of actions, so they work in a
        generic fashion. 

        Controllables have a type. For each different type, a controller
        class exists which handles the actual movement of the entity based
        on the actions a controllable may perform.
        
        Many actions will probably be common to most or all controllables,
        but actions can also be specific to a type of controllable.
        F.ex. a tank could have a unique action 'rotate turret'.

        For an example of a custom controller for an entity, see
        AvatarControllable.


Registered by RexLogic::RexLogicModule.

<b>No Attributes</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any components</b>.

</table>

*/
    class EC_Controllable : public IComponent
    {
        Q_OBJECT

        DECLARE_EC(EC_Controllable);
    public:
        virtual ~EC_Controllable() {}

        //! Add a new action for this controllable
        void AddAction(int action)
        {
            assert(actions_.find(action) == actions_.end() && "Action already added to this controller.");
            actions_.insert(action);
        }

        void RemoveAction(int action)
        {
            assert(actions_.find(action) != actions_.end() && "Action not present in this controller.");
            actions_.erase(action);
        }

        ////! Sets current action for this controllable. Event for the action is send in a delayed fashion.
        //void SetCurrentAction(int action)
        //{ 
        //    assert (action != 0 && "Action ID 0 reserved for internal use.");
        //    assert(actions_.find(action) != actions_.end() && "Action not supported by this controller.");

        //    current_action_ = action;

        //    dirty_ = true;
        //}

        //! Set unique name for this controllable
        void SetType(RexTypes::ControllableType type) { type_ = type; }

        //! Returns the type of this controllable
        RexTypes::ControllableType GetType() const { return type_; }

    private:
        typedef std::set<int> ActionSet;

        //! list of actions supported by this controller
        ActionSet actions_;

        //! Currently active action for this controllable
        int current_action_;

        //! If true, an action is pending and needs to be sent
        bool dirty_;

        //! Type of this controllable
        RexTypes::ControllableType type_;

        EC_Controllable(IModule* module) : IComponent(module->GetFramework()), current_action_(0), dirty_(false) {}
    };

    namespace Actions
    {
        namespace
        {
            //! A helper function for assigning common actions to a controllable. For an example of how to use, see AvatarControllable.
            /*! Returns default mappings from input events to actions which can be used for convinience by controllables.

                This function is re-entrant but not threadsafe

                \param controllable Controllable component. Not a ComponentPtr to avoid type casts
                \return A mapping from input events to controller actions
            */
            RexTypes::Actions::ActionInputMap AssignCommonActions(EC_Controllable *controllable)
            {
                using namespace RexTypes::Actions;
                controllable->AddAction(MoveForward);
                controllable->AddAction(MoveBackward);
                controllable->AddAction(MoveLeft);
                controllable->AddAction(MoveRight);
                controllable->AddAction(RotateLeft);
                controllable->AddAction(RotateRight);
                controllable->AddAction(MoveUp);
                controllable->AddAction(MoveDown);
                controllable->AddAction(RotateUp);
                controllable->AddAction(RotateDown);

                ActionInputMap input_map;

                input_map[Input::Events::MOVE_FORWARD_PRESSED] = MoveForward;
                input_map[Input::Events::MOVE_FORWARD_RELEASED] = MoveForward + 1;
                input_map[Input::Events::MOVE_BACK_PRESSED] = MoveBackward;
                input_map[Input::Events::MOVE_BACK_RELEASED] = MoveBackward + 1;
                input_map[Input::Events::MOVE_LEFT_PRESSED] = MoveLeft;
                input_map[Input::Events::MOVE_LEFT_RELEASED] = MoveLeft + 1;
                input_map[Input::Events::MOVE_RIGHT_PRESSED] = MoveRight;
                input_map[Input::Events::MOVE_RIGHT_RELEASED] = MoveRight + 1;
                input_map[Input::Events::ROTATE_LEFT_PRESSED] = RotateLeft;
                input_map[Input::Events::ROTATE_LEFT_RELEASED] = RotateLeft + 1;
                input_map[Input::Events::ROTATE_RIGHT_PRESSED] = RotateRight;
                input_map[Input::Events::ROTATE_RIGHT_RELEASED] = RotateRight + 1;

                input_map[Input::Events::MOVE_UP_PRESSED] = MoveUp;
                input_map[Input::Events::MOVE_UP_RELEASED] = MoveUp + 1;
                input_map[Input::Events::MOVE_DOWN_PRESSED] = MoveDown;
                input_map[Input::Events::MOVE_DOWN_RELEASED] = MoveDown + 1;
                input_map[Input::Events::ROTATE_UP_PRESSED] = RotateUp;
                input_map[Input::Events::ROTATE_UP_RELEASED] = RotateUp + 1;
                input_map[Input::Events::ROTATE_DOWN_PRESSED] = RotateUp;
                input_map[Input::Events::ROTATE_DOWN_RELEASED] = RotateUp + 1;

                return input_map;
            }
        }
    }
}

#endif
