// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreMovableTextOverlay_h
#define incl_OgreRenderer_EC_OgreMovableTextOverlay_h

#include "ComponentInterface.h"
#include "OgreModuleApi.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "Color.h"
#include "Declare_EC.h"

#include <OgreVector2.h>

namespace Ogre
{
    class TextAreaOverlayElement;
    class OverlayContainer;
    class Overlay;
    class SceneNode;
    class MovableObject;
    class Camera;
    class Font;
}


namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;

	
/**
<table class="header">
<tr>
<td>
<h2>OgreMovableTextOverlay</h2>
		Movable Ogre text overlay. 

Registered by OgreRenderer::OgreRenderingModule.

<b>No Attributes</b>.

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgrePlaceable</b>.
</table>
*/
    //! Movable Ogre text overlay.
    /*! \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreMovableTextOverlay : public Foundation::ComponentInterface
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreMovableTextOverlay);
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreMovableTextOverlay(Foundation::ModuleInterface* module);

        //! copy constructor. 
//        EC_OgreMovableTextOverlay(const EC_OgreMovableTextOverlay &other);

    public:
        //! destructor
        virtual ~EC_OgreMovableTextOverlay();

        //! assignment operator. See copy constructor
//        EC_OgreMovableTextOverlay &operator =(const EC_OgreTextOverlay &other) { assert (false); return *this; }

        //! Creates the Ogre overlay with a specified offset. Call first.
        void CreateOverlay(const Vector3df& offset);

        //! Adjust offset of overlay
        void SetOffset(const Vector3df& offset);

        //! Sets the placeable (scene node) which the overlay is meant to follow.
        void SetPlaceable(Foundation::ComponentPtr placeable);

        //! displays the text as is in the overlay
        void SetText(const std::string &text);

        //! returns the text of the overlay
        std::string GetText() const { return text_; }

        //! Set the overlay text font color.
        //! @param color New color.
        void SetFontColor(const Color &color) { fontColor_ = color; }

        //! hide / show the overlay
        void SetVisible(bool visible);

        //! Updates the text overlay and container position and dimension.
        void Update();

    private:
        //! Attach scene node to parent placeable node
        void AttachNode();

        //! Detach scene node to parent placeable node
        void DetachNode();

        //! return the dimensions of the text.
        Ogre::Vector2 GetTextDimensions(const std::string &text);

        //! Set alpha channel intensity for text and container material.
        //! @param distance The distance of the camera from the overlay node.
        void SetAlphaChannelIntensity(const float &distance);

        //! Set new material for the overlay container.
        //! @param material_name Name of the Ogre material.
        //! \todo Make this public when it works ok.
        void SetMaterial(const std::string& material_name);

        //! Overlay element for the text.
        Ogre::TextAreaOverlayElement *text_element_;

        //! container for overlay text.
        Ogre::OverlayContainer *container_;

        //! Overlay for the text.
        Ogre::Overlay *overlay_;

        //! SceneNode for the overlay text position.
        Ogre::SceneNode *node_;

        //! Parent placeable which will be followed
        Foundation::ComponentPtr placeable_;

        //! Attached to parent-flag
        bool attached_;

        //! Unique object name for the overlay.
        std::string overlayName_;

        //! Unique object name for the overlay container.
        std::string containerName_;

        //! Name of the base material used by the overlay container.
        std::string baseMaterialName_;

        //! Name of the actual material used by the overlay container.
        std::string materialName_;

        //! Max alpha value of the overlay container material.
        float materialMaxAlpha_;

        //! Whether or not the container material has alpha channel.
        bool materialHasAlpha_;

        //! The overlay text.
        std::string text_;

        //! Font which the overlay text uses.
        Ogre::Font *font_;

        //! Overlay text dimensions.
        Ogre::Vector2 textDim_;

        //! Color of the font.
        Color fontColor_;

        //! Font height
//        const float char_height_;

        //! Is the text visible
        bool visible_;

        //! The renderer, need reference to check if renderer is still alive.
        RendererWeakPtr renderer_;

        //! The window width during the overlay construction.
        int windowWidth_;

        //! The window height during the overlay construction.
        int windowHeight_;
    };
}

#endif
