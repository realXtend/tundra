// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreConsoleOverlay_h
#define incl_OgreRenderer_EC_OgreConsoleOverlay_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "CoreThread.h"

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
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;

    //! Interface for the console overlay, so in SupportModules we don't need to link against OgreRenderingModule.
    class EC_OgreConsoleOverlayInterface : public Foundation::ComponentInterface
    {
    public:
        EC_OgreConsoleOverlayInterface(Foundation::Framework *framework) :
            Foundation::ComponentInterface(framework)
        {
        }
        
        virtual ~EC_OgreConsoleOverlayInterface() {}

        virtual void Display(const std::string &text) = 0;
        virtual size_t GetMaxVisibleLines() const = 0;
        virtual void SetVisible(bool visible) = 0;
        virtual bool IsVisible() const = 0;
        virtual bool IsActive() const = 0;
        virtual void Update(f64 frametime) = 0;
    };
    
    //! Ogre overlay for debug console
    /*! \ingroup OgreRenderingModuleClient
     */
    class EC_OgreConsoleOverlay : public EC_OgreConsoleOverlayInterface
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreConsoleOverlay);
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreConsoleOverlay(IModule* module);

        //! copy constructor. 
        /*! Should not be used in practice, as only  one console overlay is supported
        */
        EC_OgreConsoleOverlay(const EC_OgreConsoleOverlay &other);

    public:
        //! destructor
        virtual ~EC_OgreConsoleOverlay();

        //! assignment operator. See copy constructor
        EC_OgreConsoleOverlay &operator ==(const EC_OgreConsoleOverlay &other) { assert (false); return *this; }

        //! displays the text as is in the overlay
        //! \todo not threadsafe, needs render mutex -cm
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
        void Update(f64 frametime);
        
    private:
        //! Create the Ogre overlay
        void CreateOverlay();

        //! Overlay element for the console
        Ogre::TextAreaOverlayElement *overlay_element_;

        //! container for overlay
        Ogre::OverlayContainer *container_;

        //! Overlay for the console
        Ogre::Overlay *overlay_;

        //! Height of the console overlay
        const float height_;

        //! font height
        const float char_height_;

        //! is the console visible
        bool visible_;

        //! is console active, i.e. accepts input
        bool active_;

        //! maximum lines the console can display. depends on height and font size
        const size_t max_visible_lines_;

        //! mutex for the console
        Mutex mutex_;

        //! console position for animation
        float position_;

        //! animation speed
        const float speed_;

        //! the renderer, need reference to check if renderer is still alive
        RendererWeakPtr renderer_;
    };
}

#endif
