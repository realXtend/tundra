// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicInputState_h
#define incl_RexLogicInputState_h

namespace Input
{
    namespace Events
    {
        class Movement;
    }
}

namespace RexLogic
{
    //! abstract interface for input states
    /*! Can be used by controller-type of classes, such as avatar controller or camera controller.
    */
    class InputStateInterface
    {
    public:
        virtual ~InputStateInterface() {}

        // Movement functions
        virtual void StartMovingForward() {}
        virtual void StartMovingBackward() {}
        virtual void StartMovingLeft() {}
        virtual void StartMovingRight() {}
        virtual void StartMovingUp() {}
        virtual void StartMovingDown() {}
        virtual void StartRotatingLeft() {}
        virtual void StartRotatingRight() {}
        virtual void StartRotatingUp() {}
        virtual void StartRotatingDown() {}
        virtual void StartRollingLeft() {}
        virtual void StartRollingRight() {}
        virtual void StopMovingForward() {}
        virtual void StopMovingBackward() {}
        virtual void StopMovingLeft() {}
        virtual void StopMovingRight() {}
        virtual void StopMovingUp() {}
        virtual void StopMovingDown() {}
        virtual void StopRotatingLeft() {}
        virtual void StopRotatingRight() {}
        virtual void StopRotatingUp() {}
        virtual void StopRotatingDown() {}
        virtual void StopRollingLeft() {}
        virtual void StopRollingRight() {}
        virtual void ToggleFlyMode() {}
        
        //! mouselook drag.
        /*!
            \param movement contains absolute and relative movement for the drag
        */
        virtual void Drag(const Input::Events::Movement *movement) {}
        virtual void Zoom(int value) {}

        //! update the state, called every frame
        virtual void Update(Core::f64 frametime) {}
    };

    typedef InputStateInterface InputState;
}

#endif
