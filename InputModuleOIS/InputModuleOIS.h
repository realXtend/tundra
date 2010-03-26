// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputModuleOIS_h
#define incl_InputModuleOIS_h

#include "OISKeyboard.h"
#include "OISMouse.h"

#include "ModuleInterface.h"
#include "InputEvents.h"
#include "InputModuleApi.h"
#include "InputServiceInterface.h"

#include <QFlags>
#include <QTime>
#include <QPair>

namespace Foundation
{
   class Framework;
}

//! OIS input library, external.
namespace OIS
{
    class InputManager;
    class Keyboard;
    class Mouse;
    class JoyStick;
}

/*! \defgroup Input_group Input Client Interface
    \copydoc Input
*/

//! Input related functionality. May be keyboard, mouse, game controllers or anything.
/*!
    See \ref Input_page "Handling input", to see how input events work for keyboard, mouse and joysticks.
*/
namespace Input
{
    class BufferedKeyboard;
    class Mapper;
    typedef boost::shared_ptr<BufferedKeyboard> BufferedKeyboardPtr;
    typedef boost::shared_ptr<Mapper> MapperPtr;

    //! Input slider types. \ingroup Input_group
    enum Slider
    {
        //! standard mouse dragging
        SliderMouse,
        //! Joystick sliders
        SliderJoy1
    };

    enum Modifier
    {
        None = 0x0,
        Shift = OIS::Keyboard::Shift,
        Ctrl = OIS::Keyboard::Ctrl,
        Alt = OIS::Keyboard::Alt,
        All = 0x0001000
     };


    //! info for key event. Unique by keycode / modifier pair. 
    /*! Different keys can launch the same event, but same key can't launch multiple events.
        \todo Refactor, rename UnBufferedKeyEventInfo into something more appropriate as it handles mouse and joystick buttons too. 
              Merge RegisterUnbufferedKeyEvent() and RegisterMouseButtonEvent().
    */
    struct UnBufferedKeyEventInfo
    {
        enum Type { Keyboard, Mouse, Joystick };
        // compiler generated copy constructor applies

        //! comparison
        bool operator ==(const UnBufferedKeyEventInfo &rhs) const
        { 
            return (key_ == rhs.key_ && type_ == rhs.type_ && 
                (modifier_ == rhs.modifier_ || pressed_event_id_ == rhs.pressed_event_id_ || released_event_id_ == rhs.released_event_id_ )  
                );
        }

        event_id_t pressed_event_id_; //! event that is launched when the key is pressed down
        event_id_t released_event_id_; //! event that is launched when the key is released
        int key_; //! ois keycode
        QFlags<Modifier> modifier_; //! modifier key
        bool pressed_; //! is the key currently pressed down
        Type type_; //! type of the key, keyboard key, mouse button, joystick button
    };

    //! info for input "sliders" (f.ex. mouse dragging)
    struct SliderEventInfo
    {
        // compiler generated copy constructor applies

        //! comparison
        bool operator ==(const SliderEventInfo &rhs) const
        { 
            return (slider_ == rhs.slider_ && 
                (modifier_ == rhs.modifier_  || dragged_event_ == rhs.dragged_event_ || stopped_event_ == rhs.stopped_event_)
                );
        }

        event_id_t dragged_event_;
        event_id_t stopped_event_;
        Slider slider_;
        int button_;
        //! modifiers
        QFlags<Modifier> modifier_;
        //! true if slider is currently dragged, false otherwise
        bool dragged_;
        //! current slider absolute and relative position
        Events::Movement movement_;
    };


    //! Input module that uses OIS for input. 
    /*! One should not work with raw OIS key codes or events under any circumstances, as that will cause too many problems.
        Instead, raw OIS events and codes are translated into generic input events which are launched when the corresponding
        input event happens, such as a key is pressed down.
        These input events can then be handled like any other event.

        See \ref Input_page "Handling input" for more information.

        \note OIS is used in unbuffered mode, so doesn't work well for UI input. See BufferedKeyboard for UI input.

        \todo Joysticks / gamepads not handled yet -cm
    */
    class  InputModuleOIS : public Foundation::ModuleInterfaceImpl, public OIS::KeyListener
    {
    public:
        typedef std::vector<UnBufferedKeyEventInfo> KeyEventInfoVector;
        typedef std::map<Input::State, KeyEventInfoVector> KeyEventInfoMap;

        typedef std::vector<SliderEventInfo> SliderInfoVector;
        typedef std::map<Input::State, SliderInfoVector> SliderInfoMap;
		
        InputModuleOIS();
        virtual ~InputModuleOIS();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update(f64 frametime);

        virtual bool HandleEvent(event_category_id_t category_id, event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! Returns if conditions for the event are true.
        /*! This is alternative method of handling input, instead of handling events,
            you can use this function to poll the state directly.
        */
        bool IsEvent(event_id_t input_event) const;

        //! returns true if key with specified keycode is currently held down. Internal use only!
        INPUTOIS_MODULE_API bool IsKeyDown(OIS::KeyCode keycode) const;

        //! returns true if button with specified code is currently held down. Internal use only!
        INPUTOIS_MODULE_API bool IsButtonDown(OIS::MouseButtonID code) const;

        //! returns slider movement info matching the specified event, but only if the slider is currently being dragged
        /*! If more than one slider matching the event is being dragged, one is chosen arbitrarily.
        */
        INPUTOIS_MODULE_API boost::optional<const Events::Movement&> GetDraggedSliderInfo(event_id_t dragged_event);

        //! returns true if slider corresponding to the event is dragged
        //bool IsDragged(event_id_t slider_event) const;

        //! Polls the current mouse state for both absolute and relative movement
        /*! Not thread safe. Internal use only!
        */
        INPUTOIS_MODULE_API const Events::Movement &GetMouseMovement() const;// { return movement_; }

        //! add a key for unbuffered listening
        /*! Internal use only!

            precond: pressed_event + 1 == released_event

            \note Not efficient.

            \param state The state for which to register the event for
            \param key OIS keycode that launches the event. See OIS for the key codes
            \param pressed_event event that is launched when key is pressed (launched once)
            \param released_event event that is launched when key is released (launched once)
            \param modifier bit flag for modifier keys (ctrl, shift, alt). See OIS for the bit flag values.
        */
        void RegisterUnbufferedKeyEvent(Input::State state, OIS::KeyCode key, event_id_t pressed_event, event_id_t released_event, int modifier = 0);

        //! add a mouse button for listening
        /*! Internal use only!

            precond: pressed_event + 1 == released_event

            \note Not efficient.

            \param state The state for which to register the event for
            \param button OIS mouse button that launches the event. See OIS for the codes
            \param pressed_event event that is launched when button is pressed (launched once)
            \param released_event event that is launched when button is released (launched once)
            \param modifier bit flag for modifier keys (ctrl, shift, alt). See OIS for the bit flag values.
        */
        void RegisterMouseButtonEvent(Input::State state, OIS::MouseButtonID button, event_id_t pressed_event, event_id_t released_event, int modifier = 0);

        //! Register a slider input
        /*! Internal use only!

            \note parameter stopped_event is not currently used for anything, but it's reserved for future use

            \param state The state for which to register the event for
            \param slider Slider type, Mouse, joystick...
            \param button Optional button on the device that needs to be held down
            \param dragged_event event for dragging the slider (launches as long as slider is dragged)
            \param stopped_event event that is launched when the slider is no longer dragged (launched once)
        */
        void RegisterSliderEvent(Input::State state, Slider slider, event_id_t dragged_event, event_id_t stopped_event, int button = -1, int modifier = 0);

        //! Set current input state
        INPUTOIS_MODULE_API void SetState(Input::State state);

        //! Returns current input state
        INPUTOIS_MODULE_API Input::State GetState() const { return keyboard_->buffered() ? Input::State_Buffered : input_state_; }

        //! Introspection of registered key events. Internal use only!
        const KeyEventInfoMap &GetRegisteredKeyEvents() const { return listened_keys_; }

        //! Introspection of registered slider events. Internal use only!
        const SliderInfoMap &GetRegisteredSliderEvents() const { return sliders_; }

        //! OIS callback for buffered keyboard
        virtual bool keyPressed( const OIS::KeyEvent &arg );
        //! OIS callback for buffered keyboard
        virtual bool keyReleased( const OIS::KeyEvent &arg );

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Input;
    private:
        //! Check h/w input events and update any events accordingly.
        /*! Similar to SendKeyEvents, but does not send any actual events,
            since event handling for sliders is a bit different.
        */
        void UpdateSliderEvents(Input::State state);

        //! Checks keyboard input and sends input events based on if any keys are pressed.
        void SendKeyEvents(Input::State state);

        //! See if keys corresponding to the event are held down
        bool IsEvent(const UnBufferedKeyEventInfo &info) const;

        //! Handle closing the main window
        /*! Should only be called when main window gets closed, not f.ex. when additional render windows get closed.
        */
        void WindowClosed();

        //! Handle window resizing, only for the main render window
        void WindowResized(int width, int height);

        //! Returns key event info vector for state state
        KeyEventInfoVector &GetKeyInfo(Input::State state);

        //! Returns key event info vector for state state
        const KeyEventInfoVector &GetKeyInfo(Input::State state) const;

        //! Returns key event info vector for state state
        SliderInfoVector &GetSliderInfo(Input::State state);

        //! input event category
        event_category_id_t event_category_;

        //! OIS objects
        OIS::InputManager *input_manager_;
        OIS::Keyboard *keyboard_;
        OIS::Mouse *mouse_;
        OIS::JoyStick *joy_;
        
        //! unbuffered keys for which pressed and released events are created. Good for movement keys, etc.
        KeyEventInfoMap listened_keys_;

        //! input sliders
        SliderInfoMap sliders_;

        //! mappings for h/w input to generic events
        MapperPtr key_mapping_;

        //! for GetMouseMovement()
        Events::Movement movement_;

        //! is mouse dragged
        bool dragged_;

        //! current input state
        Input::State input_state_;

        //! vector of key states, used for sending raw keycode events. Plain array should be fine.
        bool key_states_[256];

        bool repeat_key_;
        int last_key_code_;
        uint last_key_text_;

        int multipleKeyLimit_;
        QTime keyTimer_;

        QList<QPair<int, uint> > pressedKeys_;
    };
}
#endif
