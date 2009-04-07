// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreConsoleOverlay_h
#define incl_OgreRenderer_EC_OgreConsoleOverlay_h


#include "ComponentInterface.h"
#include "Foundation.h"

namespace Ogre
{
    class TextAreaOverlayElement;
    class OverlayContainer;
    class Overlay;
}

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! Ogre mesh component
    /*! Needs to be attached to a placeable (aka scene node) to be visible.
     */
    class MODULE_API EC_OgreConsoleOverlay : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreConsoleOverlay);
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreConsoleOverlay(Foundation::ModuleInterface* module);

        //! copy constructor. Should not be used in practice, as only 
        //! one console overlay is supported
        EC_OgreConsoleOverlay(const EC_OgreConsoleOverlay &other);

    public:
        //! destructor
        virtual ~EC_OgreConsoleOverlay();

        //! assignment operator. See copy constructor
        EC_OgreConsoleOverlay &operator ==(const EC_OgreConsoleOverlay &other) { assert (false); return *this; }

        //! Print out the specified string
        void Print(const std::string &text);

        //! update console
        __inline void Update(Core::f64 frametime)
        {
            if (visible_)
                UpdateInternal(frametime);
        }
        
    private:
        //! Create the Ogre overlay
        void CreateOverlay();

        //! Displays buffer's content on the overlay. Only displays max_visible_lines_ lines.
        void DisplayCurrentBuffer();

        //! for blinking cursor and maybe some other stuff
        void UpdateInternal(Core::f64 frametime);

        //! Overlay element for the console
        Ogre::TextAreaOverlayElement *overlay_element_;

        //! container for overlay
        Ogre::OverlayContainer *container_;

        //! Overlay for the console
        Ogre::Overlay *overlay_;

        //! Ogre renderer
        const RendererPtr renderer_;

        //! Height of the console overlay
        const Core::Real height_;

        //! font height
        const Core::Real char_height_;

        //! is the console visible
        bool visible_;

        //! Contains all lines printed to the console
        Core::StringList message_lines_;

        //! maximum lines the console can display. depends on height and font size
        const size_t max_visible_lines_;

        //! maximum number of lines the console will buffer
        const size_t max_lines_;

        //! mutex for the console
        Core::Mutex mutex_;
    };
}

#endif
