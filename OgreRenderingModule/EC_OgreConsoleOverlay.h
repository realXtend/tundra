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
    
    //! Ogre overlay for debug console
    /*! \ingroup OgreRenderingModuleClient
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

        //! displays the text as is in the overlay
        //! \todo not threadsafe, needs render mutex
        void Display(const std::string &text);

        //! Returns the max number of lines that can be visible at ones
        size_t GetMaxVisibleLines() const { return max_visible_lines_; }

        //! hide / show the overlay
        void SetVisible(bool visible);

        //! Returns true if overlay is visible, false otherwise
        /*! \note returns actual visibility of the overlay. May return true shortly after 
                  calling SetVisible(false), because the overlay might still be animating
                  and is not hidden immediatelly.
        */
        bool IsVisible() const;

        //! Returns true if console is accepting input, false otherwise
        bool IsActive() const { return active_; }

        //! visual effects
        void Update(Core::f64 frametime);
        
    private:
        //! Create the Ogre overlay
        void CreateOverlay();

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

        //! is console active, i.e. accepts input
        bool active_;

        //! maximum lines the console can display. depends on height and font size
        const size_t max_visible_lines_;

        //! mutex for the console
        Core::Mutex mutex_;

        //! console position for animation
        Core::Real position_;

        //! animation speed
        const Core::Real speed_;
    };
}

#endif
