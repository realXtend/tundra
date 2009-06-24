// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCustomObject_h
#define incl_OgreRenderer_EC_OgreCustomObject_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OgreModuleApi.h"

namespace Ogre
{
    class ManualObject;
    class Entity;
}

namespace OgreRenderer
{
    class Renderer;
    class EC_OgrePlaceable;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! Ogre custom object component
    /*! Needs to be attached to a placeable (aka scene node) to be visible.
        Note that internally this converts the manual object to a mesh entity because of render queue bugs in Ogre
        related to manual objects (still unfixed as of 1.6.2)
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
        void SetPlaceable(Foundation::ComponentPtr placeable, Scene::Entity *parent_entity = NULL);
        
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
        
        //! Commit changes
        /*! converts ManualObject to mesh, makes an entity out of it & clears the manualobject.
            \return true if successful
         */
        bool CommitChanges();
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreCustomObject(Foundation::ModuleInterface* module);
        
        //! attaches entity to placeable
        void AttachEntity();
        
        //! detaches entity from placeable
        void DetachEntity();
        
        //! removes old entity and mesh
        void DestroyEntity();
        
        //! placeable component 
        Foundation::ComponentPtr placeable_;
        
        //! renderer
        RendererPtr renderer_;
        
        //! Ogre manual object
        Ogre::ManualObject* object_;
        
        //! Ogre mesh entity (converted from the manual object on commit)
        Ogre::Entity* entity_;
        
        //! object attached to placeable -flag
        bool attached_;
        
        //! whether should cast shadows
        bool cast_shadows_;
        
        //! draw distance
        float draw_distance_;
        
        //! parent entity
        Scene::Entity* parent_entity_;
    };
}

#endif