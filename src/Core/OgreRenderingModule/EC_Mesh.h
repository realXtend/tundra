// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "Geometry/OBB.h"
#include "Geometry/AABB.h"
#include "Transform.h"
#include "AssetReference.h"
#include "AssetRefListener.h"

#include <QVector3D>

namespace Ogre
{
    class Bone;
}

/// Ogre mesh entity component
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
<li>AssetReference: meshRef
<div>Mesh asset reference (handles resource request automatically).</div>
<li>AssetReference: skeletonRef
<div>Skeleton asset reference (handles resource request automatically).</div>
<li>AssetReferenceList: meshMaterial
<div>Mesh material asset reference list, material requests are handled automatically.</div> 
<li>float: drawDistance
<div>Distance where the mesh is shown from the camera, 0.0 = draw always (default).</div> 
<li>bool: castShadows
<div>Will the mesh cast shadows.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"SetPlaceable": sets placeable component
        set a null placeable to detach the object, otherwise will attach
        @param placeable placeable component
<li>"SetDrawDistance": sets draw distance
        @param draw_distance New draw distance, 0.0 = draw always (default)
<li>"SetMesh":sets mesh
        if mesh already sets, removes the old one
        @param mesh_name mesh to use
        @param clone whether mesh should be cloned for modifying geometry uniquely
        @return true if successful 
<li>"SetMeshWithSkeleton": sets mesh with custom skeleton
        if mesh already sets, removes the old one
        @param mesh_name mesh to use
        @param skeleton_name skeleton to use
        @param clone whether mesh should be cloned for modifying geometry uniquely
        @return true if successful
<li>"SetMaterial": sets material in mesh
        @param index submesh index
        @param material_name material name
        @return true if successful
<li>"SetAdjustPosition": sets adjustment (offset) position
        @param position new position
<li>"SetAdjustOrientation": sets adjustment orientation
        @param orientation new orientation
<li>"SetAdjustScale": sets adjustment scale
        @param position new scale
<li>"RemoveMesh": removes mesh
<li>"SetAttachmentMesh": sets an attachment mesh
      The mesh entity must exist before attachment meshes can be set. Setting a new mesh entity removes all attachment meshes.
      @param index attachment index starting from 0.
      @param mesh_name mesh to use
      @param attach_point bone in entity's skeleton to attach to. if empty or nonexistent, attaches to entity root
      @param share_skeleton whether to link animation (for attachments that are also skeletally animated)
      @return true if successful 
<li>"SetAttachmentPosition": sets position of attachment mesh, relative to attachment poiont
<li>"SetAttachmentOrientation": sets orientation of attachment mesh, relative to attachment point
<li>"SetAttachmentScale": sets scale of attachment mesh, relative to attachment point
<li>"RemoveAttachmentMesh": removes an attachment mesh
      @param index attachment index starting from 0
<li>"RemoveAllAttachments": removes all attachments
<li>"SetAttachmentMaterial": sets material on an attachment mesh
     @param index attachment index starting from 0
    @param submesh_index submesh in attachment mesh
     @param material_name material name
     @return true if successful 
<li>"HasMesh": returns if mesh exists
<li>"GetNumAttachments": returns number of attachments
        note: returns just the size of attachment vector, so check individually that attachments actually exist
<li>"HasAttachmentMesh": returns if attachment mesh exists
<li>"GetPlaceable": gets placeable component
<li>"GetMeshName": returns mesh name
<li>"GetSkeletonName": returns mesh skeleton name
<li>"GetEntity": returns Ogre mesh entity 
<li>"GetAttachmentEntity": returns Ogre attachment mesh entity
<li>"GetNumMaterials": gets number of materials (submeshes) in mesh entity
<li>"GetAttachmentNumMaterials": gets number of materials (submeshes) in attachment mesh entity
<li>"GetMatName": gets material name from mesh
    @param index submesh index
    @return name if successful, empty if no entity / illegal index
<li>"GetAttachmentMaterialNam": gets material name from attachment mesh
     @param index attachment index
     @param submesh_index submesh index
     @return name if successful, empty if no entity / illegal index
<li>"GetBoundingBox": returns bounding box of Ogre mesh entity
        /// returns zero box if no entity
<li>"GetAdjustPosition": returns adjustment position
<li>"GetAdjustOrientation": returns adjustment orientation
<li>"GetAdjustScale": returns adjustment scale
<li>"GetAttachmentPosition": returns offset position of attachment
<li>"GetAttachmentOrientation": returns offset orientation of attachment
<li>"GetAttachmentScale": returns offset scale of attachment
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
    COMPONENT_NAME("EC_Mesh", 17)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Mesh(Scene* scene);

    virtual ~EC_Mesh();

    /// Transformation attribute is used to do some position, rotation and scale adjustments.
    Q_PROPERTY(Transform nodeTransformation READ getnodeTransformation WRITE setnodeTransformation);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, nodeTransformation);

    /// Mesh resource id is a asset id for a mesh resource that user wants to apply (Will handle resource request automatically).
    Q_PROPERTY(AssetReference meshRef READ getmeshRef WRITE setmeshRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, meshRef);

    /// Skeleton asset reference (handles resource request automatically).
    Q_PROPERTY(AssetReference skeletonRef READ getskeletonRef WRITE setskeletonRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, skeletonRef);

    /// Mesh material asset reference list, material requests are handled automatically.
    Q_PROPERTY(AssetReferenceList meshMaterial READ getmeshMaterial WRITE setmeshMaterial);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReferenceList, meshMaterial);

    /// Mesh draw distance, 0.0 = draw always (default)
    Q_PROPERTY(float drawDistance READ getdrawDistance WRITE setdrawDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, drawDistance);

    /// Will the mesh cast shadows.
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);

public slots:
    /// Automatically finds the placeable from the parent entity and sets it.
    void AutoSetPlaceable();

    /// Sets placeable component
    /** Set a null placeable to detach the object, otherwise will attach
        @param placeable placeable component */
    void SetPlaceable(ComponentPtr placeable);
    /// @todo override for pythonqt & qtscript, the shared_ptr issue strikes again; remove if/when possible.
    void SetPlaceable(EC_Placeable* placeable);

    /// Sets mesh
    /** if mesh already sets, removes the old one
        @param meshResourceName The name of the mesh resource to use. This will not initiate an asset request, but assumes 
            the mesh already exists as a loaded Ogre resource.
        @param clone whether mesh should be cloned for modifying geometry uniquely.
        @return true if successful. */
    bool SetMesh(QString meshResourceName, bool clone = false);

    /// Sets mesh with custom skeleton
    /** if mesh already sets, removes the old one
        @param mesh_name mesh to use
        @param skeleton_name skeleton to use
        @param clone whether mesh should be cloned for modifying geometry uniquely
        @return true if successful */
    bool SetMeshWithSkeleton(const std::string& mesh_name, const std::string& skeleton_name, bool clone = false);

    /// Sets material in mesh
    /** @param index submesh index
        @param material_name material name. Note that the material name must only contain ASCII characters! Even though
            this string is a QString, Unicode or other character encodings are not supported by Ogre.
        @return true if successful */
    bool SetMaterial(uint index, const QString& material_name);

    /// (Re)applies the currently set material refs to the currently set mesh ref.
    /** Does not start any asset requests, but sets the data on the currently loaded assets. */
    void ApplyMaterial();

    /// Sets adjustment (offset) position
    /** @param position new position */
    void SetAdjustPosition(const float3& position);

    /// Sets adjustment orientation
    /** @param orientation new orientation */
    void SetAdjustOrientation(const Quat &orientation);

    /// Sets adjustment scale
    /** @param position new scale */
    void SetAdjustScale(const float3& scale);

    /// Removes mesh
    void RemoveMesh();

    /// Sets an attachment mesh.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        The mesh entity must exist before attachment meshes can be set. Setting a new mesh entity removes all attachment meshes.
        @param index attachment index starting from 0.
        @param mesh_name mesh to use
        @param attach_point bone in entity's skeleton to attach to. if empty or nonexistent, attaches to entity root
        @param share_skeleton whether to link animation (for attachments that are also skeletally animated)
        @return true if successful */
    bool SetAttachmentMesh(uint index, const std::string& mesh_name, const std::string& attach_point = std::string(), bool share_skeleton = false);

    /// Sets position of attachment mesh, relative to attachment point.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void SetAttachmentPosition(uint index, const float3& position);

    /// Sets orientation of attachment mesh, relative to attachment point.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void SetAttachmentOrientation(uint index, const Quat &orientation);

    /// Sets scale of attachment mesh, relative to attachment point.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void SetAttachmentScale(uint index, const float3& scale);
    
    /// Removes an attachment mesh.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @param index attachment index starting from 0 */
    void RemoveAttachmentMesh(uint index);

    /// Removes all attachments.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void RemoveAllAttachments();

    /// Sets material on an attachment mesh.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @param index attachment index starting from 0
        @param submesh_index submesh in attachment mesh
        @param material_name material name
        @return true if successful */
    bool SetAttachmentMaterial(uint index, uint submesh_index, const std::string& material_name);

    /// Returns number of attachments.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @note returns just the size of attachment vector, so check individually that attachments actually exist */
    uint GetNumAttachments() const { return attachment_entities_.size(); }

    /// Returns Ogre attachment mesh entity.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    Ogre::Entity* GetAttachmentEntity(uint index) const;

    /// Returns number of materials (submeshes) in attachment mesh entity.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    uint GetAttachmentNumMaterials(uint index) const;

    /// gets material name from attachment mesh.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @param index attachment index
        @param submesh_index submesh index
        @return name if successful, empty if no entity / illegal index */
    const std::string& GetAttachmentMaterialName(uint index, uint submesh_index) const;

    /// Returns if mesh exists
    bool HasMesh() const { return entity_ != 0; }

    /// Returns number of submeshes
    /** @return returns 0 if mesh is not set, otherwise will ask Ogre::Mesh the submesh count. */
    uint GetNumSubMeshes() const;

    /// Returns if attachment mesh exists at specific @c index
    bool HasAttachmentMesh(uint index) const;

    /// Returns placeable component
    ComponentPtr GetPlaceable() const { return placeable_; }

    /// Returns mesh name
    const std::string& GetMeshName() const;

    /// Returns mesh skeleton name
    const std::string& GetSkeletonName() const;

    /// Returns Ogre mesh entity
    Ogre::Entity* GetEntity() const { return entity_; }

    /// Returns number of materials (submeshes) in the mesh entity
    uint GetNumMaterials() const;
    
    //! returns an Ogre bone safely, or null if not found.
    Ogre::Bone* GetBone(const QString& boneName) const;
    
    /// Gets material name from mesh
    /** @param index submesh index
        @return name if successful, empty if no entity / illegal index */
    const std::string& GetMaterialName(uint index) const;
    ///\todo Remove, leave just one.
    QString GetMatName(uint index) const { return GetMaterialName(index).c_str(); }

    /// Returns adjustment position
    float3 GetAdjustPosition() const;

    /// Returns adjustment orientation
    Quat GetAdjustOrientation() const;

    /// Returns adjustment scale
    float3 GetAdjustScale() const;

    /// Returns offset position of attachment
    float3 GetAttachmentPosition(uint index) const;

    /// Returns offset orientation of attachment
    Quat GetAttachmentOrientation(uint index) const;

    /// Returns offset scale of attachment
    float3 GetAttachmentScale(uint index) const;

    /// Returns adjustment scene node (used for scaling/offset/orientation modifications)
    Ogre::SceneNode* GetAdjustmentSceneNode() const { return adjustment_node_; }

    /// Returns the affine transform that maps from the local space of this mesh to the space of the EC_Placeable component of this mesh.
    /** If the Entity this mesh is part of does not have an EC_Placeable component, returns the local->world transform of this mesh instance. */
    float3x4 LocalToParent() const;

    /// Returns the affine transform that maps from the local space of this mesh to the world space of the scene.
    float3x4 LocalToWorld() const;

    /// Returns the parent component of this component in the scene transform hierarchy.
//    virtual IComponent *GetParentComponent();

    /// 
//    float3x4 IComponent::GetWorldTransform();

    /// Helper for setting asset ref from js with less code (and at all from py, due to some trouble with assetref decorator setting)
    ///\todo Remove when abovementioned problems are resolved.
    void SetMeshRef(const AssetReference& newref) { setmeshRef(newref); }
    ///\todo Remove when abovementioned problems are resolved.
    void SetMeshRef(const QString& newref) { setmeshRef(AssetReference(newref)); }

    /// Return Ogre bone safely
    Ogre::Bone* GetBone(const QString& bone_name);

    /// Return names of all bones. If no entity or skeleton, returns empty list
    QStringList GetAvailableBones() const;

    /// Force a skeleton update. Call this before GetBonePosition()... to make sure bones have updated positions, even if the mesh is currently invisible
    void ForceSkeletonUpdate();
    /// Return bone's local position
    float3 GetBonePosition(const QString& bone_name);
    /// Return bone's root-derived position. Note: these are not world coordinates, but relative to the mesh root
    float3 GetBoneDerivedPosition(const QString& bone_name);
    /// Return bone's local orientation
    Quat GetBoneOrientation(const QString& bone_name);
    /// Return bone's root-derived orientation
    Quat GetBoneDerivedOrientation(const QString& bone_name);
    /// Return bone's local orientation as Euler degrees
//    float3 GetBoneOrientationEuler(const QString& bone_name);
    /// Return bone's root-derived orientation as Euler degrees
//    float3 GetBoneDerivedOrientationEuler(const QString& bone_name);
    
    /// Set the weight (0.0 - 1.0) of a morph on the mesh
    void SetMorphWeight(const QString& morphName, float weight);
    /// Return the weight of a morph on the mesh. Returns 0.0 if not found
    float GetMorphWeight(const QString& morphName) const;
    
    /// Set the weight of a morph on an attachment mesh
    void SetAttachmentMorphWeight(unsigned index, const QString& morphName, float weight);
    /// Return the weight of a morph on an attachment mesh. Return 0.0 if not found
    float GetAttachmentMorphWeight(unsigned index, const QString& morphName) const;

    /// Returns the world space bounding box of this object.
    OBB WorldOBB() const;

    /// Returns the local space bounding box of this object.
    OBB LocalOBB() const;

    /// Returns the world space axis-aligned bounding box of this object.
    AABB WorldAABB() const;

    /// Returns the local space axis-aligned bounding box of this object.
    AABB LocalAABB() const;

public:
    /// Raycast into an Ogre mesh entity using a world-space ray. Returns true if a hit happens, in which case the fields (which are not null) are filled appropriately
    static bool Raycast(Ogre::Entity* meshEntity, const Ray& ray, float* distance = 0, unsigned* subMeshIndex = 0, unsigned* triangleIndex = 0, float3* hitPosition = 0, float3* normal = 0, float2* uv = 0);
    
signals:
    /// Emitted before the Ogre mesh entity is about to be destroyed
    void MeshAboutToBeDestroyed();

    /// Signal is emitted when mesh has successfully loaded and applied to entity.
    void MeshChanged();

    /// Signal is emitted when material has successfully applied to sub mesh.
    void MaterialChanged(uint index, const QString &material_name);

    /// Signal is emitted when skeleton has successfully applied to entity.
    void SkeletonChanged(QString skeleton_name);
    
private slots:
    /// Called when the parent entity has been set.
    void UpdateSignals();

    /// Called when some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute *attribute);

    /// Called when component has been removed from the parent entity. Checks if the component removed was the placeable, and autodissociates it.
    void OnComponentRemoved(IComponent* component, AttributeChange::Type change);

    /// Called when mesh asset has been downloaded.
    void OnMeshAssetLoaded(AssetPtr mesh);

    /// Called when skeleton asset has been downloaded.
    void OnSkeletonAssetLoaded(AssetPtr skeleton);

    /// Called when material asset has been downloaded.
    void OnMaterialAssetLoaded(AssetPtr material);

    /// Called when loading a material asset failed
    void OnMaterialAssetFailed(IAssetTransfer* transfer, QString reason);

private:
    /// Prepares a mesh for creating an entity. some safeguards are needed because of Ogre "features"
    /** @param mesh_name Mesh to prepare
        @param clone Whether should return an uniquely named clone of the mesh, rather than the original
        @return pointer to mesh, or 0 if could not be safely prepared */
    Ogre::Mesh* PrepareMesh(const std::string& mesh_name, bool clone = false);

    /// attaches entity to placeable
    void AttachEntity();

    /// detaches entity from placeable
    void DetachEntity();

    /// placeable component 
    ComponentPtr placeable_;

    /// Ogre world ptr
    OgreWorldWeakPtr world_;

    /// Ogre mesh entity
    Ogre::Entity* entity_;

    /// Attachment entities
    std::vector<Ogre::Entity*> attachment_entities_;
    /// Attachment nodes
    std::vector<Ogre::Node*> attachment_nodes_;

    /// non-empty if a cloned mesh is being used; should be removed when mesh is removed
    std::string cloned_mesh_name_;

    /// adjustment scene node (scaling/offset/orientation modifications)
    Ogre::SceneNode* adjustment_node_;

    /// mesh entity attached to placeable -flag
    bool attached_;

    /// Manages material asset requests for EC_Mesh. This utility object is used so that EC_Mesh also gets notifications about
    /// changes to material assets on disk.
    std::vector<AssetRefListenerPtr> materialAssets;

    /// Manages mesh asset requests for EC_Mesh.
    AssetRefListenerPtr meshAsset;

    /// Manages skeleton asset requests for EC_Mesh.
    AssetRefListenerPtr skeletonAsset;

    std::map<int, QString> pendingMaterialApplies;
};
