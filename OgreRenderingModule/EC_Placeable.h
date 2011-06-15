// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "IAttribute.h"
#include "EntityReference.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Transform.h"
#include "Math/float3.h"
#include "Math/MathFwd.h"

namespace Ogre { class Bone; }

/// Ogre placeable (scene node) component
/**
<table class="header">
<tr>
<td>
<h2>Placeable</h2>
Ogre (scene node) component.

Registered by OgreRenderer::OgreRenderingModule.

<b>Attributes</b>:
<ul>
<li>Transform: transform
<div>Sets the position, rotation and scale of the entity.</div>
<li>bool: drawDebug
<div>Shows the debug bounding box of geometry attached to the placeable.</div>
<li>int: selectionLayer
<div>Selection layer for raycasts.</div>
<li>EntityReference: parentRef
<div>The entity to attach to. The entity in question needs to have EC_Placeable as well to work correctly</div>
<li>string: parentBone
<div>The bone to attach to. The parent entity needs to have a skeletal EC_Mesh component</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows the Entity
<li>"Hide": Hides the Entity
<li>"ToggleVisibility": Toggles visibility
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>"ShowEntity": Shows the Entity
<li>"HideEntity": Hides the Entity
<li>"ToggleEntity": Toggles visibility
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any components</b>.

</table>
*/
class OGRE_MODULE_API EC_Placeable : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Placeable", 20)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Placeable(Scene* scene);
    virtual ~EC_Placeable();

    /// Stores the position, rotation and scale of this sceene node in the coordinate space of its parent.
    Q_PROPERTY(Transform transform READ gettransform WRITE settransform);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, transform);

    /// If true, the bounding box of the mesh in this entity is shown (for debugging purposes).
    Q_PROPERTY(bool drawDebug READ getdrawDebug WRITE setdrawDebug);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, drawDebug);

    /// Specifies whether any objects attached to the scene node of this placeable are visible or not.
    Q_PROPERTY(bool visible READ getvisible WRITE setvisible);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, visible);

    /// Specifies the selection layer of this object. This can be used to selectively perform raycasts.
    Q_PROPERTY(int selectionLayer READ getselectionLayer WRITE setselectionLayer)
    DEFINE_QPROPERTY_ATTRIBUTE(int, selectionLayer);

    /// Specifies the parent entity of this entity. Set to 0 for no parenting.
    Q_PROPERTY(EntityReference parentRef READ getparentRef WRITE setparentRef)
    DEFINE_QPROPERTY_ATTRIBUTE(EntityReference, parentRef);

    /// Specifies the name of the bone on the parent entity.
    /// Needs that the parent entity has a skeletal mesh. 
    /// Set to empty for no parent bone assignment, in which case this scene node is attached to the root of the parent node.
    Q_PROPERTY(QString parentBone READ getparentBone WRITE setparentBone)
    DEFINE_QPROPERTY_ATTRIBUTE(QString, parentBone);

    /// Returns the Ogre scene node for attaching geometry.
    /** Do not manipulate the pos/orientation/scale of this node directly, but instead use the Transform property. */
    Ogre::SceneNode* GetSceneNode() const { return sceneNode_; }

public slots:
    /// Sets the translation part of this placeable's transform.
    /// @note This function sets the Transform attribute of this component, and synchronizes to network.
    void SetPosition(float x, float y, float z);
    void SetPosition(const float3 &pos);

    /// Sets the orientation of this placeable's transform.
    /// If you want to set the orientation of this placeable using Euler angles, use e.g. 
    /// the Quat::FromEulerZYX function.
    /// @note This function sets the Transform attribute of this component, and synchronizes to network.
    void SetOrientation(const Quat &q);

    void SetScale(float x, float y, float z);
    void SetScale(const float3 &scale);

    /// Sets the rotation and scale of this placeable (the local-to-parent transform).
    /// @param mat The transformation matrix to set. This matrix is assumed to be orthogonal (no shear), 
    ///            and can not contain any mirroring.
    /// @note This function sets the Transform attribute of this component, and synchronizes to network.
    void SetTransform(const float3x3 &mat);
    /// Sets the position, rotation and scale of this placeable (the local-to-parent transform).
    /// @param tm An orthogonal matrix (no shear), which cannot contain mirroring. The float4x4 version is provided
    ///           for conveniency, and the last row must be identity [0 0 0 1].
    /// @note This function sets the Transform attribute of this component, and synchronizes to network.
    void SetTransform(const float3x4 &tm);
    void SetTransform(const float4x4 &tm);

    /// Returns the position of this placable node in world space.
    float3 WorldPosition() const;

    /// Returns the orientation of this placeable node in world space.
    Quat WorldOrientation() const;

    /// Returns the scale of this placeable node in world space.
    float3 WorldScale() const;

    /// Returns the position of this placeable node in the space of its parent.
    float3 Position() const;

    /// Returns the orientation of this placeable node in the space of its parent.
    Quat Orientation() const;

    /// Returns the scale of this placeable node in the space of its parent.
    float3 Scale() const;

    /// Returns the concatenated world transformation of this placeable.
    float3x4 LocalToWorld() const;
    /// Returns the matrix that transforms objects from world space into the local coordinate space of this placeable.
    float3x4 WorldToLocal() const;

    /// Returns the local transformation of this placeable in the space of its parent.
    /// @note For a placeable which is not attached to any parent, this returns the same transform as LocalToWorld().
    float3x4 LocalToParent() const;
    /// Returns the matrix that transforms objects from this placeable's parent's space into the local coordinate 
    /// space of this placeable.
    /// @note For a placeable which is not attached to any parent, this returns the same transform as WorldToLocal().
    float3x4 ParentToLocal() const;

    /// Shows the entity
    /** @note Doesn't alter the component's visible attribute. */
    void Show();

    /// Hides the entity.
    /** @note Doesn't alter the component's visible attribute. */
    void Hide();

    /// Toggles visibility.
    /** @note Doesn't alter the component's visible attribute. */
    void ToggleVisibility();

    /// Return whether is attached to the Ogre scene node hierarchy
    bool IsAttached() const { return attached_; }

signals:
    /// Emitted when about to be destroyed
    void AboutToBeDestroyed();

private slots:
    /// Handle attributechange
    /** @param attribute Attribute that changed.
        @param change Change type. */
    void HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

    /// Handle destruction of the parent placeable
    void OnParentPlaceableDestroyed();
    
    /// Handle destruction of the parent mesh
    void OnParentMeshDestroyed();
    
    /// Handle late creation of the parent entity, and try attaching to it
    void CheckParentEntityCreated(Entity* entity, AttributeChange::Type change);
    
    /// Handle change of the parent mesh
    void OnParentMeshChanged();
    
    /// Handle a component being added to the parent entity, in case it is the missing component we need
    void OnComponentAdded(IComponent* component, AttributeChange::Type change);

private:
    /// attaches scenenode to parent
    void AttachNode();
    
    /// detaches scenenode from parent
    void DetachNode();
    
    /// Ogre world ptr
    OgreWorldWeakPtr world_;
    
    /// Ogre scene node for geometry. This always exists as long as the EC_Placeable is alive
    Ogre::SceneNode* sceneNode_;
    
    /// Ogre scene node for manual bone attachment
    Ogre::SceneNode* boneAttachmentNode_;

    /// The bone we are tracking in bone attachment mode
    Ogre::Bone* parentBone_;
    
    /// Parent placeable, if any
    EC_Placeable* parentPlaceable_;
    
    /// Parent mesh in bone attachment mode
    EC_Mesh* parentMesh_;

    /// attached to scene hierarchy-flag
    bool attached_;

    friend class BoneAttachmentListener;
    friend class CustomTagPoint;
};
