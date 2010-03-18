// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCustomObject_h
#define incl_OgreRenderer_EC_OgreCustomObject_h

#include "ComponentInterface.h"
#include "OgreModuleApi.h"
#include "Declare_EC.h"

#include "Vector3D.h"

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
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;

    //! Ogre custom object component
    /*! Needs to be attached to a placeable (aka scene node) to be visible.
        Note that internally this converts the manual object to a mesh entity because of render queue bugs in Ogre
        related to manual objects (still unfixed as of 1.6.2)
        \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreCustomObject : public Foundation::ComponentInterface
    {
        Q_OBJECT

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

        //! Commit changes
        /*! converts ManualObject to mesh, makes an entity out of it & clears the manualobject.
            \return true if successful
         */
        bool CommitChanges();

        //! Sets material on already committed geometry, similar to EC_OgreMesh
        /*! \param index submesh index
            \param material_name material name
            \return true if successful
         */
        bool SetMaterial(uint index, const std::string& material_name);

        //! gets number of materials (submeshes) in committed geometry
        uint GetNumMaterials() const;

        //! gets material name from committed geometry
        /*! \param index submesh index
            \return name if successful, empty if not committed / illegal index
         */
        const std::string& GetMaterialName(uint index) const;

        //! Returns true if geometry has been committed and mesh entity created
        bool IsCommitted() const { return entity_ != 0; }

        void GetBoundingBox(Vector3df& min, Vector3df& max) const;

        //! Returns the Ogre entity.
        Ogre::Entity *GetEntity() const { return entity_; }

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
        RendererWeakPtr renderer_;
        
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
    };
}

#endif