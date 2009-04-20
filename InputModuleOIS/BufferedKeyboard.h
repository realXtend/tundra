// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputBufferedKeyboard_h
#define incl_InputBufferedKeyboard_h

namespace Input
{
    class InputModuleOIS;

    //! Propagates buffered keyboard input events.
    /*! Usually input needs to be unbuffered for lag-free movement,
        but for example gui input should be buffered, otherwise
        characters might get dropped and cause frustration for the
        end user. Also it might not be possible to get key codes as
        'text' that can be injected into gui element directly from
        unbuffered keyboard.

        \todo The problem is, we have buffered keyboard which launches
              events directly with OIS keycodes, and then we have
              unbuffered keyboard that also launches events.

              When buffered keyboard launches an event corresponding
              to keycode A, unbuffered keyboard should not launch it's
              own event that matches the keycode A. We can have two
              different events launched by single key stroke, but that
              should not happen (the avatar might move when typing into
              a gui widget, f.ex.).
    */
    class BufferedKeyboard : OIS::KeyListener
    {
        BufferedKeyboard();
    public:
        BufferedKeyboard(InputModuleOIS *module);
        virtual ~BufferedKeyboard();

        void Update()
        {
            keyboard_->capture();
        }

        //! Returns true if input event matching the ois key code was handled
        //! This is to resolve problems between buffered and unbuffered keyboard events
        bool IsKeyHandled(OIS::KeyCode key);

        //! OIS callback
        virtual bool keyPressed( const OIS::KeyEvent &arg );
        //! OIS callback
        virtual bool keyReleased( const OIS::KeyEvent &arg );

    private:
        typedef std::map<int, bool> KeyMap;

        Foundation::Framework *framework_;
        //! parent module
        InputModuleOIS *module_;

        //! input event category
        Core::event_category_id_t event_category_;

        // OIS objects
        OIS::InputManager *input_manager_;
        OIS::Keyboard *keyboard_;

        KeyMap handled_;
    };
}

#endif

