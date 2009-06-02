// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCustomObject_h
#define incl_OgreRenderer_EC_OgreCustomObject_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OgreModuleApi.h"

namespace Ogre
{
    class ManualObject;
}

namespace OgreRenderer
{
    class Renderer;
    class EC_OgrePlaceable;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! Ogre custom object component
    /*! Needs to be attached to a placeable (aka scene node) to be visible.
        \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreCustomObject : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreCustomObject);
    public:
        virtual ~EC_OgreCustomObject();

        //! gets placeable component
        Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! sets placeable component
        /*! set a null placeable to detach the object, otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(Foundation::ComponentPtr placeable);
        
        //! sets draw distance
        /*! \param draw_distance New draw distance, 0.0 = draw always (default)
         */
        void SetDrawDistance(float draw_distance);
                
        //! Sets if the object casts shadows or not.
        void SetCastShadows(bool enabled);
        
        //! returns the custom object
        /*! use the Ogre::ManualObject interface to actually create geometry.
         */
        Ogre::ManualObject* GetObject() const { return object_; }
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreCustomObject(Foundation::ModuleInterface* module);
        
        //! attaches object to placeable
        void AttachObject();
        
        //! detaches object from placeable
        void DetachObject();
        
        //! placeable component 
        Foundation::ComponentPtr placeable_;
        
        //! renderer
        RendererPtr renderer_;
        
        //! Ogre manual object
        Ogre::ManualObject* object_;
        
        //! object attached to placeable -flag
        bool attached_;
    };
}

#endif