// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreMesh_h
#define incl_OgreRenderer_EC_OgreMesh_h

#include "ComponentInterface.h"
#include "OgreModuleApi.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "Declare_EC.h"

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
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
    
    //! Ogre mesh entity component
    /*! Needs to be attached to a placeable (aka scene node) to be visible.
        \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreMesh : public Foundation::ComponentInterface
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreMesh);
    public:
        virtual ~EC_OgreMesh();
    public slots:    
        //! sets placeable component
        /*! set a null placeable to detach the object, otherwise will attach
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
            \param clone whether mesh should be cloned for modifying geometry uniquely
            \return true if successful
         */
        bool SetMesh(const std::string& mesh_name, bool clone = false);
        bool SetMesh(const QString& mesh_name); //same as above, just for PythonQt compatibility

        //! sets mesh with custom skeleton
        /*! if mesh already sets, removes the old one
            \param mesh_name mesh to use
            \param skeleton_name skeleton to use
            \param clone whether mesh should be cloned for modifying geometry uniquely
            \return true if successful
         */
        bool SetMeshWithSkeleton(const std::string& mesh_name, const std::string& skeleton_name, bool clone = false);

        //! sets material in mesh
        /*! \param index submesh index
            \param material_name material name
            \return true if successful
         */
        bool SetMaterial(uint index, const std::string& material_name);
        bool SetMaterial(uint index, const QString& material_name);
        //! sets adjustment (offset) position
        /*! \param position new position
         */
        void SetAdjustPosition(const Vector3df& position);
        
        //! sets adjustment orientation
        /*! \param orientation new orientation
         */
        void SetAdjustOrientation(const Quaternion& orientation);
                
        //! sets adjustment scale
        /*! \param position new scale
         */
        void SetAdjustScale(const Vector3df& scale);
        
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
        bool SetAttachmentMesh(uint index, const std::string& mesh_name, const std::string& attach_point = std::string(), bool share_skeleton = false);
        
        //! sets position of attachment mesh, relative to attachment poiont
        void SetAttachmentPosition(uint index, const Vector3df& position);
        
        //! sets orientation of attachment mesh, relative to attachment point
        void SetAttachmentOrientation(uint index, const Quaternion& orientation);
        
        //! sets scale of attachment mesh, relative to attachment point
        void SetAttachmentScale(uint index, const Vector3df& scale);
        
        //! removes an attachment mesh
        /*! \param index attachment index starting from 0
         */
        void RemoveAttachmentMesh(uint index);
        
        //! removes all attachments
        void RemoveAllAttachments();
        
        //! sets material on an attachment mesh
        /*! \param index attachment index starting from 0
            \param submesh_index submesh in attachment mesh
            \param material_name material name
            \return true if successful
         */
        bool SetAttachmentMaterial(uint index, uint submesh_index, const std::string& material_name);
        
        //! returns if mesh exists
        bool HasMesh() const { return entity_ != 0; }
        
        //! returns number of attachments
        /*! note: returns just the size of attachment vector, so check individually that attachments actually exist
         */
        uint GetNumAttachments() const { return attachment_entities_.size(); }
        
        //! returns if attachment mesh exists
        bool HasAttachmentMesh(uint index) const;
        
        //! gets placeable component
        Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! returns mesh name
        const std::string& GetMeshName() const;

        //! returns mesh skeleton name
        const std::string& GetSkeletonName() const;
        
        //! Sets if the entity casts shadows or not.
        void SetCastShadows(bool enabled);
        
        //! returns Ogre mesh entity
        Ogre::Entity* GetEntity() const { return entity_; }

       //! returns Ogre attachment mesh entity
        Ogre::Entity* GetAttachmentEntity(uint index) const;

        //! gets number of materials (submeshes) in mesh entity
        uint GetNumMaterials() const;
        
        //! gets number of materials (submeshes) in attachment mesh entity
        uint GetAttachmentNumMaterials(uint index) const;
        
        //! gets material name from mesh
        /*! \param index submesh index
            \return name if successful, empty if no entity / illegal index
         */
        const std::string& GetMaterialName(uint index) const;
        const QString& GetMatName(uint index);
        //! gets material name from attachment mesh
        /*! \param index attachment index
            \param submesh_index submesh index
            \return name if successful, empty if no entity / illegal index
         */
        const std::string& GetAttachmentMaterialName(uint index, uint submesh_index) const;

        //! returns bounding box of Ogre mesh entity
        //! returns zero box if no entity
        void GetBoundingBox(Vector3df& min, Vector3df& max) const;
        
        //! returns adjustment position
        Vector3df GetAdjustPosition() const;
        //! returns adjustment orientation
        Quaternion GetAdjustOrientation() const;
        //! returns adjustment scale
        Vector3df GetAdjustScale() const;
        
        //! returns offset position of attachment
        Vector3df GetAttachmentPosition(uint index) const;
        //! returns offset orientation of attachment
        Quaternion GetAttachmentOrientation(uint index) const;
        //! returns offset scale of attachment
        Vector3df GetAttachmentScale(uint index) const;
        
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
        RendererWeakPtr renderer_;
        
        //! Ogre mesh entity
        Ogre::Entity* entity_;
        
        //! Attachment entities
        std::vector<Ogre::Entity*> attachment_entities_;
        //! Attachment nodes
        std::vector<Ogre::Node*> attachment_nodes_;
        
        //! non-empty if a cloned mesh is being used; should be removed when mesh is removed
        std::string cloned_mesh_name_;
        
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
