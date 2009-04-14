// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputModuleOIS_h
#define incl_InputModuleOIS_h

#include "ModuleInterface.h"

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
    typedef boost::shared_ptr<BufferedKeyboard> BufferedKeyboardPtr;

    //! Input module that uses OIS for input. OIS is used in unbuffered mode, so doesn't work well for UI input.
    class InputModuleOIS : public Foundation::ModuleInterfaceImpl
    {
    public:
        InputModuleOIS();
        virtual ~InputModuleOIS();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update(Core::f64 frametime);

        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

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
    };
}
#endif
