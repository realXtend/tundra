// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreMesh_h
#define incl_OgreRenderer_EC_OgreMesh_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OgreModuleApi.h"

namespace Ogre
{
    class Entity;
    class SceneNode;
}

namespace OgreRenderer
{
    class Renderer;
    class EC_OgrePlaceable;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! Ogre mesh entity component
    /*! Needs to be attached to a placeable (aka scene node) to be visible.
        \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreMesh : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreMesh);
    public:
        virtual ~EC_OgreMesh();

        //! gets placeable component
        Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! sets placeable component
        /*! set a null placeable to detach the mesh, otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(Foundation::ComponentPtr placeable);
        
        //! sets mesh
        /*! if mesh already sets, removes the old one
            \param mesh_name mesh to use
            \return true if successful
         */
        bool SetMesh(const std::string& mesh_name);
        
        //! gets number of materials (submeshes) in mesh entity
        Core::uint GetNumMaterials();
    
        //! gets material name from mesh
        /*! \param index submesh index           
            \return name if successful, empty if no entity / illegal index
         */
        const std::string& GetMaterialName(Core::uint index);
                
        //! sets material in mesh
        /*! \param index submesh index
            \param material_name material name
            \return true if successful
         */
        bool SetMaterial(Core::uint index, const std::string& material_name);
        
        //! sets adjustment (offset) position
        /*! \param position new position
         */
        void SetAdjustPosition(const Core::Vector3df& position);
        
        //! sets adjustment orientation
        /*! \param orientation new orientation
         */
        void SetAdjustOrientation(const Core::Quaternion& orientation);
                
        //! sets adjustment scale
        /*! \param position new scale
         */
        void SetAdjustScale(const Core::Vector3df& scale);
        
        //! removes mesh
        void RemoveMesh();
        
        //! returns if mesh exists
        bool HasMesh() const { return entity_ != NULL; }
        
        //! returns mesh name
        const std::string& GetMeshName() const;
        
        //! returns Ogre mesh entity
        Ogre::Entity* GetEntity() const { return entity_; }
        
        //! returns bounding box of Ogre mesh entity
        //! returns zero box if no entity
        void GetBoundingBox(Core::Vector3df& min, Core::Vector3df& max);
        
        //! returns adjustment position
        Core::Vector3df GetAdjustPosition() const;
        //! returns adjustment orientation
        Core::Quaternion GetAdjustOrientation() const;
        //! returns adjustment scale
        Core::Vector3df GetAdjustScale() const;
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreMesh(Foundation::ModuleInterface* module);
        
        //! attaches entity to placeable
        void AttachEntity();
        
        //! detaches entity from placeable
        void DetachEntity();
        
        //! scales entity, using adjustment node, according to "scale to unity" setting
        void ScaleEntity();
        
        //! placeable component 
        Foundation::ComponentPtr placeable_;
        
        //! renderer
        RendererPtr renderer_;
        
        //! Ogre mesh entity
        Ogre::Entity* entity_;
        
        //! Adjustment scene node (scaling/offset/orientation modifications)
        Ogre::SceneNode* adjustment_node_;
        
        //! mesh entity attached to placeable -flag
        bool attached_;
    };
}

#endif
