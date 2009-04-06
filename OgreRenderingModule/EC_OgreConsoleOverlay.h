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

    public:
        //! destructor
        virtual ~EC_OgreConsoleOverlay();

        //! Print out the specified string
        virtual void Print(const std::string &text);
        
    private:
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
    };
}

#endif
