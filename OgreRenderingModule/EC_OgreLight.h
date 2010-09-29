// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreLight_h
#define incl_OgreRenderer_EC_OgreLight_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Color.h"
#include "Vector3D.h"
#include "Declare_EC.h"

namespace OgreRenderer
{
//! Ogre light component
/**
<table class="header">
<tr>
<td>
<h2>OgreLight</h2>

A light can optionally be attached to a placeable (ie. a scene node) but it can also exist without one.

Registered by OgreRenderer::OgreRenderingModule.

\ingroup OgreRenderingModuleClient

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
    class OGRE_MODULE_API EC_OgreLight : public IComponent
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreLight);
    public:
        //! light type enumeration
        enum Type
        {
            LT_Point,
            LT_Spot,
            LT_Directional
        };
        
        //! Destructor.
        virtual ~EC_OgreLight();
    
        //! gets placeable component
        ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! sets placeable component
        /*! set a null placeable (or do not set a placeable) to have a detached light
            \param placeable placeable component
         */
        void SetPlaceable(ComponentPtr placeable);
        
        //! sets type of light
        /*! \param type light type - point, directional or spot
         */
        void SetType(Type type);
        
        //! sets diffuse color of light
        /*! \param color diffuse color value
         */
        void SetColor(const Color& color);

        //! sets light attenuation parameters
        /*! \param range maximum range of light
            \param constant constant attenuation
            \param linear linear attenuation
            \param quad quadratic attenuation
         */
        void SetAttenuation(float range, float constant, float linear, float quad);
        
        //! sets light direction
        /*! does not affect point lights
            \param direction light direction
         */
        void SetDirection(const Vector3df& direction);
        
        //! Whether the light casts shadows or not.
        //! @param enabled Whether the light casts shadows or not.
        void SetCastShadows(const bool &enabled);

        //! @return Ogre light pointer
        Ogre::Light* GetLight() const { return light_; }
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreLight(IModule* module);
        
        //! attaches light to placeable
        void AttachLight();
        
        //! detaches light from placeable
        void DetachLight();
        
        //! placeable component, optional
        ComponentPtr placeable_;
        
        //! renderer
        RendererWeakPtr renderer_;
        
        //! Ogre light
        Ogre::Light* light_;
        
        //! light attached to placeable -flag
        bool attached_;
    };
}

#endif
