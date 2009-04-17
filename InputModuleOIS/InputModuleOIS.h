// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputModuleOIS_h
#define incl_InputModuleOIS_h

#include "OISKeyboard.h"

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

// Input related functionality. May be keyboard, mouse, game controllers or anything
namespace Input
{
    class BufferedKeyboard;
    class Mapper;
    typedef boost::shared_ptr<BufferedKeyboard> BufferedKeyboardPtr;
    typedef boost::shared_ptr<Mapper> MapperPtr;


    struct UnBufferedKeyEventInfo
    {
        Core::event_id_t Pressed_EventId;
        Core::event_id_t Released_EventId;
        OIS::KeyCode Key;
        bool bPressed;
    };


    //! Input module that uses OIS for input. OIS is used in unbuffered mode, so doesn't work well for UI input.
    //! \todo INPUTOIS_MODULE_API doesn't work for some reason
    class /* INPUTOIS_MODULE_API */ InputModuleOIS : public Foundation::ModuleInterfaceImpl
    {
    public:
        InputModuleOIS();
        virtual ~InputModuleOIS();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update(Core::f64 frametime);

        /// Provides a polling-style method for checking whether a key is down. You can use this if the stateful pressed/released event-based
        /// system is too unwieldy for your use. Use conservatively, since directly calling this from your logic module will create a 
        /// hard-to-break dependency to the InputModuleOIS, and will require some state objects to track when the input is valid to read. 
        /// (i.e. can't move using WSAD when typing into a textbox)
        /// \todo Make this not inline and properly generate INPUTOIS_MODULE_API etc.. defines for DLL export.
        bool IsKeyDown(OIS::KeyCode key) { return keyboard_ ? keyboard_->isKeyDown(key) : false; }

        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

        //! Polls the current mouse state for both absolute and relative movement
        /*! Not thread safe
        */
        __inline const Events::Movement &GetMouseMovement() const { return movement_; }

        //! add a key for unbuffered listening
        void AddUnbufferedKeyEvent(OIS::KeyCode key, Core::event_id_t pressed_event, Core::event_id_t released_event);

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
        OIS::Keyboard *keyboard_buffered_;
        OIS::Mouse *mouse_;
        OIS::JoyStick *joy_;

        //! buffered keyboard
        BufferedKeyboardPtr buffered_keyboard_;
        
        //! unbuffered keys for which pressed and released events are created. Good for movement keys, etc.
        std::vector<UnBufferedKeyEventInfo> listened_keys_;

        //! mappings for h/w input to generic events
        MapperPtr key_mapping_;

        //! for GetMouseMovement()
        Events::Movement movement_;
    };
}
#endif
