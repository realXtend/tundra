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
    <li>"Placeable": @copydoc Placeable
    <li>"MeshName": @copydoc MeshName
    <li>"SkeletonName": @copydoc SkeletonName
    <li>"NumMaterials": @copydoc NumMaterials
    <li>"MaterialName": @copydoc MatName
    <li>"WorldOBB": @copydoc WorldOBB
    <li>"LocalOBB": @copydoc LocalOBB
    <li>"WorldAABB": @copydoc WorldAABB
    <li>"LocalAABB": @copydoc LocalAABB
    <li>"MeshAsset": @copydoc MeshAsset
    <li>"MaterialAsset": @copydoc MaterialAsset
    <li>"SkeletonAsset": @copydoc SkeletonAsset
    <li>"AdjustPosition": @copydoc AdjustPosition
    <li>"AdjustOrientation": @copydoc AdjustOrientation
    <li>"AdjustScale": @copydoc AdjustScale
    <li>"AttachmentPosition": @copydoc AttachmentPosition
    <li>"AttachmentOrientation": @copydoc AttachmentOrientation
    <li>"AttachmentScale": @copydoc AttachmentScale
    <li>"AvailableBones": @copydoc AvailableBones
    <li>"BonePosition": @copydoc BonePosition
    <li>"BoneDerivedPosition": @copydoc BoneDerivedPosition
    <li>"BoneOrientation": @copydoc BoneOrientation
    <li>"BoneDerivedOrientation": @copydoc BoneDerivedOrientation
    <li>"SetMorphWeight": @copydoc SetMorphWeight
    <li>"MorphWeight": @copydoc MorphWeight
    <li>"SetAttachmentMorphWeight": @copydoc SetAttachmentMorphWeight
    <li>"AttachmentMorphWeight": @copydoc AttachmentMorphWeight
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
    /// @cond PRIVATE
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Mesh(Scene* scene);
    /// @endcond
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

    /// Returns Ogre mesh entity
    Ogre::Entity* OgreEntity() const { return entity_; }

    /// Returns an Ogre bone safely, or null if not found.
    Ogre::Bone* OgreBone(const QString& boneName) const;

    /// Returns adjustment scene node (used for scaling/offset/orientation modifications)
    Ogre::SceneNode* AdjustmentSceneNode() const { return adjustment_node_; }

    /// Returns Ogre attachment mesh entity.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    Ogre::Entity* AttachmentOgreEntity(uint index) const;

    /// Raycast into an Ogre mesh entity using a world-space ray.
    /** Returns true if a hit happens, in which case the fields (which are not null) are filled appropriately. */
    static bool Raycast(Ogre::Entity* meshEntity, const Ray& ray, float* distance = 0, unsigned* subMeshIndex = 0,
        unsigned* triangleIndex = 0, float3* hitPosition = 0, float3* normal = 0, float2* uv = 0);

    // DEPRECATED
    Ogre::Entity* GetEntity() const { return OgreEntity(); } /**< @deprecated use OgreEntity instead. @todo Add warning print. */
    Ogre::Bone* GetBone(const QString& boneName) const { return OgreBone(boneName); } /**< @deprecated use OgreBone instead. @todo Add warning print. */
    Ogre::SceneNode* GetAdjustmentSceneNode() const { return AdjustmentSceneNode(); } /**< @deprecated use AdjustmentSceneNode instead. @todo Add warning print. */
    Ogre::Entity* GetAttachmentEntity(uint index) const { return AttachmentOgreEntity(index); } /**< @deprecated use AttachmentOgreEntity instead. @todo Add warning print. */

public slots:
    /// Automatically finds the placeable from the parent entity and sets it.
    void AutoSetPlaceable();

    /// Sets placeable component
    /** Set a null placeable to detach the object, otherwise will attach
        @param placeable placeable component */
    void SetPlaceable(const ComponentPtr &placeable);
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
    uint NumSubMeshes() const;

    /// Returns if attachment mesh exists at specific @c index
    bool HasAttachmentMesh(uint index) const;

    /// Returns placeable component
    ComponentPtr Placeable() const { return placeable_; }

    /// Returns mesh name
    const std::string& MeshName() const;

    /// Returns mesh skeleton name
    const std::string& SkeletonName() const;

    /// Returns number of materials (submeshes) in the mesh entity
    uint NumMaterials() const;

    /// Gets material name from mesh
    /** @param index submesh index
        @return name if successful, empty if no entity / illegal index */
    const std::string& MaterialName(uint index) const;
    ///\todo Remove, leave just one.
    QString GetMatName(uint index) const { return GetMaterialName(index).c_str(); }

    /// Returns adjustment position
    float3 AdjustPosition() const;

    /// Returns adjustment orientation
    Quat AdjustOrientation() const;

    /// Returns adjustment scale
    float3 AdjustScale() const;

    /// Returns offset position of attachment
    float3 AttachmentPosition(uint index) const;

    /// Returns offset orientation of attachment
    Quat AttachmentOrientation(uint index) const;

    /// Returns offset scale of attachment
    float3 AttachmentScale(uint index) const;

    /// Returns the affine transform that maps from the local space of this mesh to the space of the EC_Placeable component of this mesh.
    /** If the Entity this mesh is part of does not have an EC_Placeable component, returns the local->world transform of this mesh instance. */
    float3x4 LocalToParent() const;

    /// Returns the affine transform that maps from the local space of this mesh to the world space of the scene.
    float3x4 LocalToWorld() const;

    /// Returns the parent component of this component in the scene transform hierarchy.
//    virtual IComponent *GetParentComponent();

    /// 
//    float3x4 IComponent::WorldTransform();

    /// Return names of all bones. If no entity or skeleton, returns empty list
    QStringList AvailableBones() const;

    /// Force a skeleton update. Call this before BonePosition()... to make sure bones have updated positions, even if the mesh is currently invisible
    void ForceSkeletonUpdate();
    /// Return bone's local position
    float3 BonePosition(const QString& boneName);
    /// Return bone's root-derived position. Note: these are not world coordinates, but relative to the mesh root
    float3 BoneDerivedPosition(const QString& boneName);
    /// Return bone's local orientation
    Quat BoneOrientation(const QString& boneName);
    /// Return bone's root-derived orientation
    Quat BoneDerivedOrientation(const QString& boneName);
    /// Return bone's local orientation as Euler degrees
//    float3 BoneOrientationEuler(const QString& boneName);
    /// Return bone's root-derived orientation as Euler degrees
//    float3 BoneDerivedOrientationEuler(const QString& boneName);
    
    /// Set the weight (0.0 - 1.0) of a morph on the mesh
    void SetMorphWeight(const QString& morphName, float weight);
    /// Return the weight of a morph on the mesh. Returns 0.0 if not found
    float MorphWeight(const QString& morphName) const;
    
    /// Set the weight of a morph on an attachment mesh
    void SetAttachmentMorphWeight(unsigned index, const QString& morphName, float weight);
    /// Return the weight of a morph on an attachment mesh. Return 0.0 if not found
    float AttachmentMorphWeight(unsigned index, const QString& morphName) const;

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

    // DEPRECATED
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
    size_t NumAttachments() const { return attachment_entities_.size(); }
    /// Returns number of materials (submeshes) in attachment mesh entity.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT */
    uint NumAttachmentMaterials(uint index) const;
    /// gets material name from attachment mesh.
    /** @deprecated THIS FUNCTION IS DEPRECATED. ONLY EC_AVATAR IS ALLOWED TO CALL IT
        @param index attachment index
        @param submeshIndex submesh index
        @return name if successful, empty if no entity / illegal index */
    const std::string& AttachmentMaterialName(uint index, uint submeshIndex) const;
    /// Helper for setting asset ref from js with less code (and at all from py, due to some trouble with assetref decorator setting)
    /// @todo Remove when abovementioned problems are resolved.
    /// @deprecated Use meshRef attribute.
    void SetMeshRef(const AssetReference& newref) { setmeshRef(newref); }
    /// @todo Remove when abovementioned problems are resolved.
    /// @deprecated Use meshRef attribute.
    void SetMeshRef(const QString& newref) { setmeshRef(AssetReference(newref)); }
    uint GetNumSubMeshes() const { return NumSubMeshes(); } /**< @deprecated use NumSubMeshes instead. @todo Add warning print. */
    ComponentPtr GetPlaceable() const { return Placeable(); } /**< @deprecated use Placeable instead. @todo Add warning print. */
    const std::string& GetMeshName() const { return MeshName(); } /**< @deprecated use MeshName instead. @todo Add warning print. */
    const std::string& GetSkeletonName() const { return SkeletonName(); } /**< @deprecated use SkeletonName instead. @todo Add warning print. */
    uint GetNumMaterials() const { return NumMaterials(); } /**< @deprecated use NumMaterials instead. @todo Add warning print. */
    const std::string& GetMaterialName(uint index) const { return MaterialName(index); } /**< @deprecated use MaterialName instead. @todo Add warning print. */
    float3 GetAdjustPosition() const { return AdjustPosition(); } /**< @deprecated use AdjustPosition instead. @todo Add warning print. */
    Quat GetAdjustOrientation() const { return AdjustOrientation(); } /**< @deprecated use AdjustOrientation instead. @todo Add warning print. */
    float3 GetAdjustScale() const { return AdjustScale(); } /**< @deprecated use AdjustScale instead. @todo Add warning print. */
    float3 GetAttachmentPosition(uint index) const { return AttachmentPosition(index); } /**< @deprecated use AttachmentPosition instead. @todo Add warning print. */
    Quat GetAttachmentOrientation(uint index) const { return AttachmentOrientation(index); } /**< @deprecated use AttachmentOrientation instead. @todo Add warning print. */
    float3 GetAttachmentScale(uint index) const { return AttachmentScale(index); } /**< @deprecated use AttachmentScale instead. @todo Add warning print. */
    QStringList GetAvailableBones() const { return AvailableBones(); } /**< @deprecated use AvailableBones instead. @todo Add warning print. */
    float3 GetBonePosition(const QString& boneName) { return BonePosition(boneName); } /**< @deprecated use BonePosition instead. @todo Add warning print. */
    float3 GetBoneDerivedPosition(const QString& boneName) { return BoneDerivedPosition(boneName); } /**< @deprecated use BoneDerivedPosition instead. @todo Add warning print. */
    Quat GetBoneOrientation(const QString& boneName) { return BoneOrientation(boneName); } /**< @deprecated use BoneOrientation instead. @todo Add warning print. */
    Quat GetBoneDerivedOrientation(const QString& boneName) { return BoneDerivedOrientation(boneName); } /**< @deprecated use BoneDerivedOrientation instead. @todo Add warning print. */
    float GetMorphWeight(const QString& morphName) const { return MorphWeight(morphName); } /**< @deprecated use MorphWeight instead. @todo Add warning print. */
    float GetAttachmentMorphWeight(unsigned index, const QString& morphName) const { return AttachmentMorphWeight(index, morphName); } /**< @deprecated use AttachmentMorphWeight instead. @todo Add warning print. */
    size_t GetNumAttachments() const { return NumAttachments(); } /**< @deprecated Use NumAttachments instead. @todo Add warning print. */
    uint GetAttachmentNumMaterials(uint index) const { return NumAttachmentMaterials(index); } /**< @deprecated use NumAttachmentMaterials instead. @todo Add warning print. */
    const std::string& GetAttachmentMaterialName(uint index, uint submeshIndex) const { return AttachmentMaterialName(index, submeshIndex); } /**< @deprecated use AttachmentMaterialName instead. @todo Add warning print. */

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
