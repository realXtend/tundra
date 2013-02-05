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

#include <QList>

/// Ogre mesh entity component
/** <table class="header">
    <tr>
    <td>
    <h2>Mesh</h2>
    Ogre mesh entity component
    Needs to be attached to a placeable (aka scene node) to be visible. 

    Registered by OgreRenderer::OgreRenderingModule.

    <b>Attributes</b>:
    <ul>
    <li>Transform: nodeTransformation
    <div>@copydoc nodeTransformation</div>
    <li>AssetReference: meshRef
    <div>@copydoc meshRef</div>
    <li>AssetReference: skeletonRef
    <div>@copydoc skeletonRef</div>
    <li>AssetReferenceList: meshMaterial
    <div>@copydoc meshMaterial</div>
    <li>float: drawDistance
    <div>@copydoc drawDistance</div>
    <li>bool: castShadows
    <div>@copydoc castShadows</div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"SetPlaceable": @copydoc SetPlaceable
    <li>"SetDrawDistance": @copydoc SetDrawDistance
    <li>"SetMesh": @copydoc SetMesh
    <li>"SetMeshWithSkeleton": @copydoc SetMeshWithSkeleton
    <li>"SetMaterial": @copydoc SetMaterial
    <li>"SetAdjustPosition": @copydoc SetAdjustPosition
    <li>"SetAdjustOrientation": @copydoc SetAdjustOrientation
    <li>"SetAdjustScale": @copydoc SetAdjustScale
    <li>"RemoveMesh": @copydoc RemoveMesh
    <li>"HasMesh": @copydoc HasMesh
    <li>"GetPlaceable": @copydoc GetPlaceable
    <li>"GetMeshName": @copydoc GetMeshName
    <li>"GetSkeletonName": @copydoc GetSkeletonName
    <li>"GetEntity": @copydoc GetEntity
    <li>"GetNumMaterials": @copydoc GetNumMaterials
    <li>"GetMatName": @copydoc GetMatName
    <li>"WorldOBB": @copydoc WorldOBB
    <li>"LocalOBB": @copydoc LocalOBB
    <li>"WorldAABB": @copydoc WorldAABB
    <li>"LocalAABB": @copydoc LocalAABB
    <li>"MeshAsset": @copydoc MeshAsset
    <li>"MaterialAsset": @copydoc MaterialAsset
    <li>"SkeletonAsset": @copydoc SkeletonAsset
    <li>"GetAdjustPosition": @copydoc GetAdjustPosition
    <li>"GetAdjustOrientation": @copydoc GetAdjustOrientation
    <li>"GetAdjustScale": @copydoc GetAdjustScale
    <li>"GetAttachmentPosition": @copydoc GetAttachmentPosition
    <li>"GetAttachmentOrientation": @copydoc GetAttachmentOrientation
    <li>"GetAttachmentScale": @copydoc GetAttachmentScale
    <li>"GetAdjustmentSceneNode": @copydoc GetAdjustmentSceneNode
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    <li>...
    </ul>
    </td>
    </tr>

    Does not emit any actions.

    <b>Depends on the component Placeable</b>.
    </table> */
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

    /// Mesh resource id is a asset id for a mesh resource that user wants to apply.
    Q_PROPERTY(AssetReference meshRef READ getmeshRef WRITE setmeshRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, meshRef);

    /// Skeleton asset reference.
    Q_PROPERTY(AssetReference skeletonRef READ getskeletonRef WRITE setskeletonRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, skeletonRef);

    /// Mesh material asset reference list.
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
    void SetPlaceable(EC_Placeable* placeable); /**< @overload */

    /// Sets mesh
    /** if mesh already sets, removes the old one
        @param meshResourceName The name of the mesh resource to use. This will not initiate an asset request, but assumes 
            the mesh already exists as a loaded Ogre resource.
        @param clone whether mesh should be cloned for modifying geometry uniquely.
        @return true if successful. */
    bool SetMesh(QString meshResourceName, bool clone = false);

    /// Sets mesh with custom skeleton
    /** if mesh already sets, removes the old one
        @param meshName mesh to use
        @param skeletonName skeleton to use
        @param clone whether mesh should be cloned for modifying geometry uniquely
        @return true if successful */
    bool SetMeshWithSkeleton(const std::string& meshName, const std::string& skeletonName, bool clone = false);

    /// Sets material in mesh
    /** @param index submesh index
        @param materialName material name. Note that the material name must only contain ASCII characters! Even though
            this string is a QString, Unicode or other character encodings are not supported by Ogre.
        @param change The signalling type to use for this change. Important! In order to preserve compatibility and old behavior,
                      the default changetype is Disconnected. When changing a material from script code, you will want to set this to
                      Default almost _always_.
        @note This function causes the signal MaterialChanged() to be fired, independent what the value of change is.
        @return true if successful */
    bool SetMaterial(uint index, const QString& materialName, AttributeChange::Type change = AttributeChange::Disconnected);

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

    /// Return Ogre bone safely
    Ogre::Bone* GetBone(const QString& boneName);

    /// Return names of all bones. If no entity or skeleton, returns empty list
    QStringList GetAvailableBones() const;

    /// Force a skeleton update. Call this before GetBonePosition()... to make sure bones have updated positions, even if the mesh is currently invisible
    void ForceSkeletonUpdate();
    /// Return bone's local position
    float3 GetBonePosition(const QString& boneName);
    /// Return bone's root-derived position. Note: these are not world coordinates, but relative to the mesh root
    float3 GetBoneDerivedPosition(const QString& boneName);
    /// Return bone's local orientation
    Quat GetBoneOrientation(const QString& boneName);
    /// Return bone's root-derived orientation
    Quat GetBoneDerivedOrientation(const QString& boneName);
    /// Return bone's local orientation as Euler degrees
//    float3 GetBoneOrientationEuler(const QString& boneName);
    /// Return bone's root-derived orientation as Euler degrees
//    float3 GetBoneDerivedOrientationEuler(const QString& boneName);
    
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

    /// Returns the mesh asset used by this component.
    OgreMeshAssetPtr MeshAsset() const;

    /// Returns the material asset used for the submesh at @c materialIndex.
    OgreMaterialAssetPtr MaterialAsset(int materialIndex) const;

    /// Returns the possible asset used by this component.
    OgreSkeletonAssetPtr SkeletonAsset() const;

    /// Sets an attachment mesh.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        The mesh entity must exist before attachment meshes can be set. Setting a new mesh entity removes all attachment meshes.
        @param index attachment index starting from 0.
        @param meshName mesh to use
        @param attachPoint bone in entity's skeleton to attach to. if empty or nonexistent, attaches to entity root
        @param shareSkeleton whether to link animation (for attachments that are also skeletally animated)
        @return true if successful */
    bool SetAttachmentMesh(uint index, const std::string& meshName, const std::string& attachPoint = std::string(), bool shareSkeleton = false);

    /// Sets position of attachment mesh, relative to attachment point.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void SetAttachmentPosition(uint index, const float3& position);

    /// Sets orientation of attachment mesh, relative to attachment point.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void SetAttachmentOrientation(uint index, const Quat &orientation);

    /// Sets scale of attachment mesh, relative to attachment point.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void SetAttachmentScale(uint index, const float3& scale);
    
    /// Removes an attachment mesh.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @param index attachment index starting from 0 */
    void RemoveAttachmentMesh(uint index);

    /// Removes all attachments.
    /** @note THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    void RemoveAllAttachments();

    /// Sets material on an attachment mesh.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @param index attachment index starting from 0
        @param submesh_index submesh in attachment mesh
        @param materialName material name
        @return true if successful */
    bool SetAttachmentMaterial(uint index, uint submesh_index, const std::string& materialName);

    /// Returns number of attachments.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @note returns just the size of attachment vector, so check individually that attachments actually exist */
    uint GetNumAttachments() const { return attachment_entities_.size(); }

    /// Returns Ogre attachment mesh entity.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    Ogre::Entity* GetAttachmentEntity(uint index) const;

    /// Returns number of materials (submeshes) in attachment mesh entity.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    uint GetAttachmentNumMaterials(uint index) const;

    /// gets material name from attachment mesh.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @param index attachment index
        @param submesh_index submesh index
        @return name if successful, empty if no entity / illegal index */
    const std::string& GetAttachmentMaterialName(uint index, uint submesh_index) const;

    /// Helper for setting asset ref from js with less code (and at all from py, due to some trouble with assetref decorator setting)
    /// @todo Remove when abovementioned problems are resolved.
    /// @deprecated Use meshRef attribute.
    void SetMeshRef(const AssetReference& newref) { setmeshRef(newref); }
    /// @todo Remove when abovementioned problems are resolved.
    /// @deprecated Use meshRef attribute.
    void SetMeshRef(const QString& newref) { setmeshRef(AssetReference(newref)); }

public:
    /// Raycast into an Ogre mesh entity using a world-space ray. Returns true if a hit happens, in which case the fields (which are not null) are filled appropriately
    static bool Raycast(Ogre::Entity* meshEntity, const Ray& ray, float* distance = 0, unsigned* subMeshIndex = 0, unsigned* triangleIndex = 0, float3* hitPosition = 0, float3* normal = 0, float2* uv = 0);
    
signals:
    /// Emitted before the Ogre mesh entity is about to be destroyed
    void MeshAboutToBeDestroyed();

    /// Signal is emitted when mesh has successfully loaded and applied to entity.
    void MeshChanged();

    /// Signal is emitted when material has successfully applied to sub mesh.
    void MaterialChanged(uint index, const QString &materialName);

    /// Signal is emitted when skeleton has successfully applied to entity.
    void SkeletonChanged(QString skeletonName);
    
private slots:
    /// Called when the parent entity has been set.
    void UpdateSignals();

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

    /// Called when some of the attributes has been changed.
    void AttributesChanged();

    /// Prepares a mesh for creating an entity. some safeguards are needed because of Ogre "features"
    /** @param meshName Mesh to prepare
        @param clone Whether should return an uniquely named clone of the mesh, rather than the original
        @return pointer to mesh, or 0 if could not be safely prepared */
    Ogre::Mesh* PrepareMesh(const std::string& meshName, bool clone = false);

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

    /// Tracking pending failed material applies.
    QList<uint> pendingFailedMaterials_;
};
