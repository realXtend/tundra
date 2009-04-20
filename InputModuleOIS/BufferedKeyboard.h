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
        end user.
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

        //! OIS callback
        virtual bool keyPressed( const OIS::KeyEvent &arg );
        //! OIS callback
        virtual bool keyReleased( const OIS::KeyEvent &arg );

    private:
        Foundation::Framework *framework_;
        //! parent module
        InputModuleOIS *module_;

        //! input event category
        Core::event_category_id_t event_category_;

        // OIS objects
        OIS::InputManager *input_manager_;
        OIS::Keyboard *keyboard_;
    };
}

#endif

