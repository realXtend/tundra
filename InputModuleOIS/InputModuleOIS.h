// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputModuleOIS_h
#define incl_InputModuleOIS_h

#include "OISKeyboard.h"
#include "OISMouse.h"

#include "ModuleInterface.h"
#include "InputEvents.h"
#include "InputModuleApi.h"


namespace Foundation
{
   class Framework;
}

namespace OIS
{
    class InputManager;
    class Keyboard;
    class Mouse;
    class JoyStick;
}

//! Input related functionality. May be keyboard, mouse, game controllers or anything
/*!
    See InputModuleOIS to see how input events work for keyboard, mouse and joysticks.
*/
namespace Input
{
    class BufferedKeyboard;
    class Mapper;
    typedef boost::shared_ptr<BufferedKeyboard> BufferedKeyboardPtr;
    typedef boost::shared_ptr<Mapper> MapperPtr;

    // Input slider types
    enum Slider
    {
        //! standard mouse dragging
        SliderMouse,
        //! Joystick sliders
        SliderJoy1
    };

    //! info for key event. Unique by keycode / modifier pair. 
    //! Different keys can launch the same event, but same key can't launch multiple events.
    struct UnBufferedKeyEventInfo
    {
        // compiler generated copy constructor applies

        //! comparison
        bool operator ==(const UnBufferedKeyEventInfo &rhs) const { return (key_ == rhs.key_ && modifier_ == rhs.modifier_ ); }

        Core::event_id_t pressed_event_id_; //! event that is launched when the key is pressed down
        Core::event_id_t released_event_id_; //! event that is launched when the key is released
        OIS::KeyCode key_; //! ois keycode
        int modifier_; //! modifier key
        bool pressed_; //! is the key currently pressed down
    };

    //! info for input "sliders" (f.ex. mouse dragging)
    struct SliderEventInfo
    {
        // compiler generated copy constructor applies

        //! comparison
        bool operator ==(const SliderEventInfo &rhs) const { return (slider_ == rhs.slider_ && modifier_ == rhs.modifier_ ); }

        Core::event_id_t dragged_event_;
        Core::event_id_t stopped_event_;
        Slider slider_;
        int button_;
        //! modifiers
        int modifier_;
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
        
        Keyboard events are supported in the following way:
        A single OIS::KeyCode that corresponds to an event, modifiers may be included (ctrl, alt and/or shift) as a bit flag.
        An event is launched when corresponding key is pressed together with any modifier keys.
        A corresponding event_released is launched when corresponding key or any modifier keys are released.

        Mouse movement is supported in the following way:
        Input sliders are used for continuous non-uniform input events such as mouse movement.
        A slider can be combined with a button press in the same device, such as mouse button, and a modifier key
        (ctrl, alt and/or shift). Slider events are not send as events, but need to be queried from an input service.
        See Foundation::InputServiceInterface for more information.

        See Mapper for default key configuration.

        \note OIS is used in unbuffered mode, so doesn't work well for UI input. See BufferedKeyboard for UI input.

        \todo Joysticks / gamepads not handled yet
        \todo INPUTOIS_MODULE_API doesn't work for some reason
    */
    class /* INPUTOIS_MODULE_API */ InputModuleOIS : public Foundation::ModuleInterfaceImpl
    {
    public:
        typedef std::vector<UnBufferedKeyEventInfo> KeyEventInfoVector;
        typedef std::vector<SliderEventInfo> SliderInfoVector;

        InputModuleOIS();
        virtual ~InputModuleOIS();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update(Core::f64 frametime);

        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! returns true if key with specified keycode is currently held down. Internal use only!
        bool IsKeyDown(OIS::KeyCode keycode) const;

        //! returns true if button with specified code is currently held down. Internal use only!
        bool IsButtonDown(OIS::MouseButtonID code) const;

        //! returns slider movement info matching the specified event, but only if the slider is currently being dragged
        /*! If more than one slider matching the event is being dragged, one is chosen arbitrarily.
        */
        boost::optional<const Events::Movement&> GetDraggedSliderInfo(Core::event_id_t dragged_event);

        //! returns true if slider corresponding to the event is dragged
        //bool IsDragged(Core::event_id_t slider_event) const;

        //! Polls the current mouse state for both absolute and relative movement
        /*! Not thread safe. Internal use only!
        */
        __inline const Events::Movement &GetMouseMovement() const { return movement_; }

        //! add a key for unbuffered listening
        /*! Internal use only!

            precond: pressed_event + 1 == released_event

            \note Not efficient.
            \param key OIS keycode that launches the event. See OIS for the key codes
            \param pressed_event event that is launched when key is pressed (launched once)
            \param released_event event that is launched when key is released (launched once)
            \param modifier bit flag for modifier keys (ctrl, shift, alt). See OIS for the bit flag values.
        */
        void RegisterUnbufferedKeyEvent(OIS::KeyCode key, Core::event_id_t pressed_event, Core::event_id_t released_event, int modifier = 0);

        //! Register a slider input
        /*! Internal use only!

            \note parameter stopped_event is not currently used for anything, but it's reserved for future use

            \param slider Slider type, Mouse, joystick...
            \param button Optional button on the device that needs to be held down
            \param dragged_event event for dragging the slider (launches as long as slider is dragged)
            \param stopped_event event that is launched when the slider is no longer dragged (launched once)
        */
        void RegisterSliderEvent(Slider slider, Core::event_id_t dragged_event, Core::event_id_t stopped_event, int button = -1, int modifier = 0);

        //! Introspection of registered key events. Internal use only!
        const KeyEventInfoVector &GetRegisteredKeyEvents() const { return listened_keys_; }

        //! Introspection of registered slider events. Internal use only!
        const SliderInfoVector &GetRegisteredSliderEvents() const { return sliders_; }

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Input;
    private:
        //! Handle closing the main window
        /*! Should only be called when main window gets closed, not f.ex. when additional render windows get closed.
        */
        void WindowClosed();

        //! Handle window resizing, only for the main render window
        void WindowResized(int width, int height);

        //! input event category
        Core::event_category_id_t event_category_;

        //! OIS objects
        OIS::InputManager *input_manager_;
        OIS::Keyboard *keyboard_;
        //OIS::Keyboard *keyboard_buffered_;
        OIS::Mouse *mouse_;
        OIS::JoyStick *joy_;

        //! buffered keyboard
        BufferedKeyboardPtr buffered_keyboard_;
        
        //! unbuffered keys for which pressed and released events are created. Good for movement keys, etc.
        KeyEventInfoVector listened_keys_;

        //! input sliders
        SliderInfoVector sliders_;

        //! mappings for h/w input to generic events
        MapperPtr key_mapping_;

        //! for GetMouseMovement()
        Events::Movement movement_;

        //! is mouse dragged
        bool dragged_;
    };
}
#endif
