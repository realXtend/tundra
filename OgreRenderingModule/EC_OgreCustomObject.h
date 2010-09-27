// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCustomObject_h
#define incl_OgreRenderer_EC_OgreCustomObject_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Declare_EC.h"

#include "Vector3D.h"

namespace OgreRenderer
{
//! Ogre custom object component
/**
<table class="header">
<tr>
<td>
<h2>OgreCustomObject</h2>

Needs to be attached to a placeable (aka scene node) to be visible.
Note that internally this converts the manual object to a mesh entity because of render queue bugs in Ogre
related to manual objects (still unfixed as of 1.6.2)

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
    class OGRE_MODULE_API EC_OgreCustomObject : public IComponent
    {
        Q_OBJECT

        DECLARE_EC(EC_OgreCustomObject);
    public:
        virtual ~EC_OgreCustomObject();

        //! gets placeable component
        ComponentPtr GetPlaceable() const { return placeable_; }

        //! sets placeable component
        /*! set a null placeable to detach the object, otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(ComponentPtr placeable);

        //! sets draw distance
        /*! \param draw_distance New draw distance, 0.0 = draw always (default)
         */
        void SetDrawDistance(float draw_distance);

        //! Sets if the object casts shadows or not.
        void SetCastShadows(bool enabled);

        //! Commit changes from a manual object
        /*! converts ManualObject to mesh, makes an entity out of it & clears the manualobject.
            \return true if successful
         */
        bool CommitChanges(Ogre::ManualObject* object);

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
        EC_OgreCustomObject(IModule* module);
        
        //! attaches entity to placeable
        void AttachEntity();
        
        //! detaches entity from placeable
        void DetachEntity();
        
        //! removes old entity and mesh
        void DestroyEntity();
        
        //! placeable component 
        ComponentPtr placeable_;
        
        //! renderer
        RendererWeakPtr renderer_;
        
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