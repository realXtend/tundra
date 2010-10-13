// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_Mesh_h
#define incl_OgreRenderer_EC_Mesh_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "Transform.h"
#include "Declare_EC.h"

#include <QVariant>

namespace Ogre
{
    class Mesh;
    class SceneNode;
    class Skeleton;
};

//! Ogre mesh entity component
/**
<table class="header">
<tr>
<td>
<h2>Mesh</h2>
Ogre mesh entity component
Needs to be attached to a placeable (aka scene node) to be visible. 

Registered by OgreRenderer::OgreRenderingModule.

<b>Attributes</b>:
<ul>
<li>Transform: nodePosition
<div>Transformation attribute is used to do some position, rotation and scale adjustments.</div>
<li>QString: meshResourceId
<div>Mesh resource ref is a asset id for a mesh resource that user wants to apply to scene.</div>
<li>QString: skeletonId
<div>Skeleton asset ref is a string that should contain skeleton asset id.</div>
<li>QVariantList: meshMaterial
<div>Mesh material ref is a string list that can contain x number of materials and each material is applied to.</div> 
<li>float: drawDistance
<div>Distance where the mesh is shown from the camera.</div> 
<li>bool: castShadows
<div>Will the mesh cast shadows.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"SetPlaceable": sets placeable component
        set a null placeable to detach the object, otherwise will attach
        \param placeable placeable component
<li>"SetDrawDistance": sets draw distance
        \param draw_distance New draw distance, 0.0 = draw always (default)
<li>"SetMesh":sets mesh
        if mesh already sets, removes the old one
        \param mesh_name mesh to use
        \param clone whether mesh should be cloned for modifying geometry uniquely
        \return true if successful 
<li>"SetMeshWithSkeleton": sets mesh with custom skeleton
        if mesh already sets, removes the old one
        \param mesh_name mesh to use
        \param skeleton_name skeleton to use
        \param clone whether mesh should be cloned for modifying geometry uniquely
        \return true if successful
<li>"SetMaterial": sets material in mesh
        \param index submesh index
        \param material_name material name
        \return true if successful
<li>"SetAdjustPosition": sets adjustment (offset) position
        \param position new position
<li>"SetAdjustOrientation": sets adjustment orientation
        \param orientation new orientation
<li>"SetAdjustScale": sets adjustment scale
        \param position new scale
<li>"RemoveMesh": removes mesh
<li>"SetAttachmentMesh": sets an attachment mesh
      The mesh entity must exist before attachment meshes can be set. Setting a new mesh entity removes all attachment meshes.
      \param index attachment index starting from 0.
      \param mesh_name mesh to use
      \param attach_point bone in entity's skeleton to attach to. if empty or nonexistent, attaches to entity root
      \param share_skeleton whether to link animation (for attachments that are also skeletally animated)
      \return true if successful 
<li>"SetAttachmentPosition": sets position of attachment mesh, relative to attachment poiont
<li>"SetAttachmentOrientation": sets orientation of attachment mesh, relative to attachment point
<li>"SetAttachmentScale": sets scale of attachment mesh, relative to attachment point
<li>"RemoveAttachmentMesh": removes an attachment mesh
      \param index attachment index starting from 0
<li>"RemoveAllAttachments": removes all attachments
<li>"SetAttachmentMaterial": sets material on an attachment mesh
     \param index attachment index starting from 0
    \param submesh_index submesh in attachment mesh
     \param material_name material name
     \return true if successful 
<li>"HasMesh": returns if mesh exists
<li>"GetNumAttachments": returns number of attachments
        note: returns just the size of attachment vector, so check individually that attachments actually exist
<li>"HasAttachmentMesh": returns if attachment mesh exists
<li>"GetPlaceable": gets placeable component
<li>"GetMeshName": returns mesh name
<li>"GetSkeletonName": returns mesh skeleton name
<li>"SetCastShadows": Sets if the entity casts shadows or not.
<li>"GetEntity": returns Ogre mesh entity 
<li>"GetAttachmentEntity": returns Ogre attachment mesh entity
<li>"GetNumMaterials": gets number of materials (submeshes) in mesh entity
<li>"GetAttachmentNumMaterials": gets number of materials (submeshes) in attachment mesh entity
<li>"GetMatName": gets material name from mesh
    \param index submesh index
    \return name if successful, empty if no entity / illegal index
<li>"GetAttachmentMaterialNam": gets material name from attachment mesh
     \param index attachment index
     \param submesh_index submesh index
     \return name if successful, empty if no entity / illegal index
<li>"GetBoundingBox": returns bounding box of Ogre mesh entity
        //! returns zero box if no entity
<li>"GetAdjustPosition": returns adjustment position
<li>"GetAdjustOrientation": returns adjustment orientation
<li>"GetAdjustScale": returns adjustment scale
<li>"GetAttachmentPosition": returns offset position of attachment
<li>"GetAttachmentOrientation": returns offset orientation of attachment
<li>"GetAttachmentScale": returns offset scale of attachment
<li>"GetDrawDistance": returns draw distance
<li>"GetAdjustmentSceneNode": Returns adjustment scene node (used for scaling/offset/orientation modifications)
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable</b>.
</table>
*/
class OGRE_MODULE_API EC_Mesh : public IComponent
{
    Q_OBJECT
    
    DECLARE_EC(EC_Mesh);
public:
    //! Transformation attribute is used to do some position, rotation and scale adjustments.
    //! @todo Transform attribute is not working in js need to expose it to QScriptEngine somehow.
    Q_PROPERTY(Transform nodeTransformation READ getnodeTransformation WRITE setnodeTransformation);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, nodeTransformation);

    //! Mesh resource id is a asset id for a mesh resource that user wants to apply (Will handle resource request automaticly).
    Q_PROPERTY(QString meshResourceId READ getmeshResourceId WRITE setmeshResourceId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, meshResourceId);

    //! Skeleton asset id, will handle request resource automaticly.
    Q_PROPERTY(QString skeletonId READ getskeletonId WRITE setskeletonId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, skeletonId);

    //! Mesh material id list that can contain x number of materials, material requests are handled automaticly.
    //! @todo replace std::vector to QVariantList.
    Q_PROPERTY(QVariantList meshMaterial READ getmeshMaterial WRITE setmeshMaterial);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, meshMaterial);

    //! Mesh draw distance.
    Q_PROPERTY(float drawDistance READ getdrawDistance WRITE setdrawDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, drawDistance);

    //! Will the mesh cast shadows.
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);

    //! Set component as serializable.
    /*! Note that despite this, in OpenSim worlds, the network sync will be disabled from the component,
        as mesh attributes are being transmitted through RexPrimData instead.
     */
    virtual bool IsSerializable() const { return true; }
    
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data);
    
    virtual ~EC_Mesh();
public slots:
    //! sets placeable component
    /*! set a null placeable to detach the object, otherwise will attach
        \param placeable placeable component
     */
    void SetPlaceable(ComponentPtr placeable);
    void SetPlaceable(EC_Placeable* placeable); //override for pythonqt & qtscript, the shared_ptr issue strikes again
    
    //! sets draw distance
    /*! \param draw_distance New draw distance, 0.0 = draw always (default)
     */
    void SetDrawDistance(float draw_distance);
    
    //! sets mesh
    /*! if mesh already sets, removes the old one
        \param mesh_name mesh to use. This will not initiate an asset request, but assumes the mesh already exists as a loaded Ogre resource
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
    ComponentPtr GetPlaceable() const { return placeable_; }
    
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
    const QString& GetMatName(uint index) const;

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
    float GetDrawDistance() const { return drawDistance.Get(); }

    //! Returns adjustment scene node (used for scaling/offset/orientation modifications)
    Ogre::SceneNode* GetAdjustmentSceneNode() const { return adjustment_node_; }

signals:
    //! Signal is emitted when mesh has successfully loaded and applied to entity.
    void OnMeshChanged();

    //! Signal is emitted when material has succussfully applied to sub mesh.
    void OnMaterialChanged(uint index, const QString &material_name);

    //! Signal is emitted when skeleton has successfully applied to entity.
    void OnSkeletonChanged(QString skeleton_name);

private slots:
    //! Called when the parent entity has been set.
    void UpdateSignals();

    //! Called when some of the attributes has been changed.
    void AttributeUpdated(IAttribute *attribute);
    
private:
    //! constructor
    /*! \param module renderer module
     */
    EC_Mesh(IModule* module);
    
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
    
    bool HandleResourceEvent(event_id_t event_id, IEventData* data);
    bool HandleMeshResourceEvent(event_id_t event_id, IEventData* data);
    bool HandleSkeletonResourceEvent(event_id_t event_id, IEventData* data);
    bool HandleMaterialResourceEvent(event_id_t event_id, IEventData* data);
    request_tag_t RequestResource(const std::string& id, const std::string& type);
    bool HasMaterialsChanged() const;
    
    //! placeable component 
    ComponentPtr placeable_;
    
    //! renderer
    OgreRenderer::RendererWeakPtr renderer_;
    
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
    
    typedef std::vector<request_tag_t> AssetRequestArray;
    AssetRequestArray materialRequestTags_;

    event_category_id_t resource_event_category_;

    typedef std::pair<request_tag_t, std::string> ResourceKeyPair;
    typedef boost::function<bool(event_id_t,IEventData*)> MeshEventHandlerFunction;
    typedef std::map<ResourceKeyPair, MeshEventHandlerFunction> MeshResourceHandlerMap;
    MeshResourceHandlerMap resRequestTags_;
};

#endif
