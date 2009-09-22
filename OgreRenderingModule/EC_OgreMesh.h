// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreMesh_h
#define incl_OgreRenderer_EC_OgreMesh_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OgreModuleApi.h"

namespace Ogre
{
    class Entity;
    class Mesh;
    class Node;
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
        
        //! sets placeable component
        /*! set a null placeable to detach the mesh, otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(Foundation::ComponentPtr placeable);
        
        //! sets draw distance
        /*! \param draw_distance New draw distance, 0.0 = draw always (default)
         */
        void SetDrawDistance(float draw_distance);
        
        //! sets mesh
        /*! if mesh already sets, removes the old one
            \param mesh_name mesh to use
            \param parent_entity parent entity we belong to
            \param clone whether mesh should be cloned for modifying geometry uniquely
            \return true if successful
         */
        bool SetMesh(const std::string& mesh_name, Scene::Entity *parent_entity, bool clone = false);

        //! sets mesh with custom skeleton
        /*! if mesh already sets, removes the old one
            \param mesh_name mesh to use
            \param skeleton_name skeleton to use
            \param parent_entity parent entity we belong to
            \param clone whether mesh should be cloned for modifying geometry uniquely
            \return true if successful
         */
        bool SetMeshWithSkeleton(const std::string& mesh_name, const std::string& skeleton_name, Scene::Entity *parent_entity, bool clone = false);

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
        
        //! sets an attachment mesh
        /*! The mesh entity must exist before attachment meshes can be set. Setting a new mesh entity removes all attachment meshes.
            \param index attachment index starting from 0.
            \param mesh_name mesh to use
            \param attach_point bone in entity's skeleton to attach to. if empty or nonexistent, attaches to entity root
            \param share_skeleton whether to link animation (for attachments that are also skeletally animated)
            \return true if successful
         */
        bool SetAttachmentMesh(Core::uint index, const std::string& mesh_name, const std::string& attach_point = std::string(), bool share_skeleton = false);
        
        //! sets position of attachment mesh, relative to attachment poiont
        void SetAttachmentPosition(Core::uint index, const Core::Vector3df& position);
        
        //! sets orientation of attachment mesh, relative to attachment point
        void SetAttachmentOrientation(Core::uint index, const Core::Quaternion& orientation);
        
        //! sets scale of attachment mesh, relative to attachment point
        void SetAttachmentScale(Core::uint index, const Core::Vector3df& scale);
        
        //! removes an attachment mesh
        /*! \param index attachment index starting from 0
         */
        void RemoveAttachmentMesh(Core::uint index);
        
        //! removes all attachments
        void RemoveAllAttachments();
        
        //! sets material on an attachment mesh
        /*! \param index attachment index starting from 0
            \param submesh_index submesh in attachment mesh
            \param material_name material name
            \return true if successful
         */
        bool SetAttachmentMaterial(Core::uint index, Core::uint submesh_index, const std::string& material_name);
        
        //! returns if mesh exists
        bool HasMesh() const { return entity_ != NULL; }
        
        //! returns number of attachments
        /*! note: returns just the size of attachment vector, so check individually that attachments actually exist
         */
        Core::uint GetNumAttachments() const { return attachment_entities_.size(); }
        
        //! returns if attachment mesh exists
        bool HasAttachmentMesh(Core::uint index) const;
        
        //! gets placeable component
        Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! returns mesh name
        const std::string& GetMeshName() const;
        
        //! Sets if the entity casts shadows or not.
        void SetCastShadows(bool enabled);
        
        //! returns Ogre mesh entity
        Ogre::Entity* GetEntity() const { return entity_; }

       //! returns Ogre attachment mesh entity
        Ogre::Entity* GetAttachmentEntity(Core::uint index) const;

        //! gets number of materials (submeshes) in mesh entity
        Core::uint GetNumMaterials() const;
        
        //! gets number of materials (submeshes) in attachment mesh entity
        Core::uint GetAttachmentNumMaterials(Core::uint index) const;
        
        //! gets material name from mesh
        /*! \param index submesh index
            \return name if successful, empty if no entity / illegal index
         */
        const std::string& GetMaterialName(Core::uint index) const;

        //! gets material name from attachment mesh
        /*! \param index attachment index
            \param submesh_index submesh index
            \return name if successful, empty if no entity / illegal index
         */
        const std::string& GetAttachmentMaterialName(Core::uint index, Core::uint submesh_index) const;

        //! returns bounding box of Ogre mesh entity
        //! returns zero box if no entity
        void GetBoundingBox(Core::Vector3df& min, Core::Vector3df& max) const;
        
        //! returns adjustment position
        Core::Vector3df GetAdjustPosition() const;
        //! returns adjustment orientation
        Core::Quaternion GetAdjustOrientation() const;
        //! returns adjustment scale
        Core::Vector3df GetAdjustScale() const;
        
        //! returns offset position of attachment
        Core::Vector3df GetAttachmentPosition(Core::uint index) const;
        //! returns offset orientation of attachment
        Core::Quaternion GetAttachmentOrientation(Core::uint index) const;
        //! returns offset scale of attachment
        Core::Vector3df GetAttachmentScale(Core::uint index) const;
        
        //! returns draw distance
        float GetDrawDistance() const { return draw_distance_; }
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreMesh(Foundation::ModuleInterface* module);
        
        //! prepares a mesh for creating an entity. some safeguards are needed because of Ogre "features"
        /*! \param mesh_name Mesh to prepare
            \param clone Whether should return an uniquely named clone of the mesh, rather than the original
            \return pointer to mesh, or 0 if could not be safely prepared
         */
        Ogre::Mesh* PrepareMesh(const std::string& mesh_name, bool clone = false);
        
        //! attaches entity to placeable
        void AttachEntity();
        
        //! detaches entity from placeable
        void DetachEntity();
        
        //! placeable component 
        Foundation::ComponentPtr placeable_;
        
        //! renderer
        RendererPtr renderer_;
        
        //! Ogre mesh entity
        Ogre::Entity* entity_;
        
        //! Attachment entities
        std::vector<Ogre::Entity*> attachment_entities_;
        //! Attachment nodes
        std::vector<Ogre::Node*> attachment_nodes_;
        
        //! whether a cloned mesh is being used, and should be removed when mesh is removed
        bool cloned_;
        
        //! adjustment scene node (scaling/offset/orientation modifications)
        Ogre::SceneNode* adjustment_node_;
        
        //! mesh entity attached to placeable -flag
        bool attached_;
        
        //! cast shadows flag
        bool cast_shadows_;
        
        //! draw distance
        float draw_distance_;
    };
}

#endif
