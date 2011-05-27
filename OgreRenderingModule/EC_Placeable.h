// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "IAttribute.h"
#include "EntityReference.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Transform.h"
#include "Vector3D.h"
#include "Quaternion.h"

#include <QQuaternion>
#include <QVector3D>

#include <OgreNode.h>

class EC_Mesh;

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
<li>"Translate": Translates the placeable. Modifies the transform attribute using Default changetype.
<li>"TranslateRelative": Translates the placeable relative to orientation. Modifies the transform attribute using Default changetype.
<li>"GetRelativeVector": Transforms a vector by the placeable's orientation.
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

Note: do not use the properties (Position, Scale, Orientation) below. They are deprecated and will hopefully be removed in the future.

</table>
*/
class OGRE_MODULE_API EC_Placeable : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Placeable", 20)

    friend class BoneAttachmentListener;
    friend class CustomTagPoint;

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Placeable(Scene* scene);
    virtual ~EC_Placeable();

    /// position property
    /// @note This affects internally the transform attribute, but allows QVector3D access to it
    Q_PROPERTY(QVector3D position READ GetQPosition WRITE SetQPosition)
    
    /// scale property
    /// @note This affects internally the transform attribute, but allows QVector3D access to it
    Q_PROPERTY(QVector3D scale READ GetQScale WRITE SetQScale)

    /// orientation property
    /// @note This affects internally the transform attribute, but allows QQuaternion access to it
    Q_PROPERTY(QQuaternion orientation READ GetQOrientation WRITE SetQOrientation)

    /// orientationEuler property
    /// @note This affects internally the transform attribute, but allows QVector3D euler angle (in degrees) access to it
    Q_PROPERTY(QVector3D orientationEuler READ GetQOrientationEuler WRITE SetQOrientationEuler)
    
    Q_PROPERTY(QVector3D LocalXAxis READ GetQLocalXAxis)
    Q_PROPERTY(QVector3D LocalYAxis READ GetQLocalYAxis)
    Q_PROPERTY(QVector3D LocalZAxis READ GetQLocalZAxis)
    Q_PROPERTY(float Yaw READ GetYaw)
    Q_PROPERTY(float Pitch READ GetPitch)
    Q_PROPERTY(float Roll READ GetRoll)
    
    /// Transformation attribute for position, rotation and scale adjustments.
    Q_PROPERTY(Transform transform READ gettransform WRITE settransform);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, transform);
    
    /// Show debug bounding box -attribute
    Q_PROPERTY(bool drawDebug READ getdrawDebug WRITE setdrawDebug);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, drawDebug);

    /// Specifies whether any objects attached to the scene node of this placeable are visible or not.
    Q_PROPERTY(bool visible READ getvisible WRITE setvisible);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, visible);
    
    /// Specifies selection layer for raycasts
    Q_PROPERTY(int selectionLayer READ getselectionLayer WRITE setselectionLayer)
    DEFINE_QPROPERTY_ATTRIBUTE(int, selectionLayer);

    /// Parent entity ref. 0 for no parenting
    Q_PROPERTY(EntityReference parentRef READ getparentRef WRITE setparentRef)
    DEFINE_QPROPERTY_ATTRIBUTE(EntityReference, parentRef);
    
    /// Parent entity bone name. Needs the parent entity to have a skeletal mesh. Empty for no parent bone assignment
    Q_PROPERTY(QString parentBone READ getparentBone WRITE setparentBone)
    DEFINE_QPROPERTY_ATTRIBUTE(QString, parentBone);

    /// orients to look at a point in space
    /** @param look_at point to look at
     */
    void LookAt(const Vector3df& look_at);

    /// yaws the node
    /** @param radians how many radians to yaw
     */
    void SetYaw(float radians);

    /// pitches the node
    /** @param radians how many radians to pitch
     */
    void SetPitch(float radians);

    /// rolls the node
    /** @param radians how many radians to roll
     */
    void SetRoll(float radians);

    /// get the yaw of the node
    float GetYaw() const;
    /// get the pitch of the node
    float GetPitch() const;
    /// get the roll of the node
    float GetRoll() const;

    /// Get the local X axis from the node orientation
    Vector3df GetLocalXAxis() const;
    /// Get the local Y axis from the node orientation
    Vector3df GetLocalYAxis() const;
    /// Get the local Z axis from the node orientation
    Vector3df GetLocalZAxis() const;
    /// Get the local X axis from the node orientation
    QVector3D GetQLocalXAxis() const;
    /// Get the local Y axis from the node orientation
    QVector3D GetQLocalYAxis() const;
    /// Get the local Z axis from the node orientation
    QVector3D GetQLocalZAxis() const;

    /// Returns Ogre scene node for attaching geometry.
    /** Do not manipulate the pos/orientation/scale of this node directly. */
    Ogre::SceneNode* GetSceneNode() const { return sceneNode_; }
    
    /// get node position
    QVector3D GetQPosition() const;
    void SetQPosition(QVector3D newpos);

    /// get node orientation
    QQuaternion GetQOrientation() const;
    /// set node orientation
    void SetQOrientation(QQuaternion newrot);

    /// get node orientation as euler (degrees)
    QVector3D GetQOrientationEuler() const;
    /// set node orientation as euler (degrees)
    void SetQOrientationEuler(QVector3D newrot);

    /// set node scale
    QVector3D GetQScale() const;
    /// get node scale
    void SetQScale(QVector3D newscale);

public slots:
    /// Convers placeable transformation to given EC_Placeable space.
    /// @todo Add scale conversion. This method wont change scale related to it's parent (only position and rotation are converted).
    /// @param comp EC_Placeable component that we are converting our transformation to.
    void ConvertToObjectSpace(IComponent *comp);

    /// Sets position
    /** @param position new position
     */
    void SetPosition(const Vector3df& position);

    /// returns position
    Vector3df GetPosition() const;

    /// sets scale
    /** @param scale new scale
     */
    void SetScale(const Vector3df& scale);

    /// returns scale
    Vector3df GetScale() const;

    /// sets orientation
    /** @param orientation new orientation
     */
    void SetOrientation(const Quaternion& orientation);

    /// sets orientation using euler angles. 
    /** @param eulerVec euler rotations.
     */
    void SetOrientation(const Vector3df& euler);

    /// returns orientation
    Quaternion GetOrientation() const;

    /// LookAt wrapper that accepts a QVector3D for py & js e.g. camera use
    void LookAt(const QVector3D look_at) { LookAt(Vector3df((f32)look_at.x(), (f32)look_at.y(), (f32)look_at.z())); }
    
    /// Translates the placeable. Modifies the transform attribute using Default changetype.
    void Translate(const Vector3df& translation);
    
    /// Translates the placeable relative to local orientation. Modifies the transform attribute using Default changetype.
    void TranslateRelative(const Vector3df& translation);
    
    /// Transforms a vector by the placeable's local orientation
    Vector3df GetRelativeVector(const Vector3df& vec);
    
    /// Translates the placeable relative to world orientation. Modifies the transform attribute using Default changetype.
    void TranslateWorldRelative(const Vector3df& translation);
    
    /// Transforms a vector by the placeable's world orientation
    Vector3df GetWorldRelativeVector(const Vector3df& vec);
    
    /// Calculates rotation
    Vector3df GetRotationFromTo(const Vector3df& from, const Vector3df& to);

    /// Return world-derived position from the scene node
    Vector3df GetWorldPosition() const;
    
    /// Return world-derived orientation from the scene node
    Quaternion GetWorldOrientation() const;
    
    /// Return world-derived orientation as euler degrees from the scene node
    Vector3df GetWorldOrientationEuler() const;
    
    /// Return world-derived scale from the scene node
    Vector3df GetWorldScale() const;

    /// Shows the entity
    /** @note Doesn't alter the component's attribute. */
    void Show();

    /// Hides the entity.
    /** @note Doesn't alter the component's visible" attribute. */
    void Hide();

    /// Toggles visibility.
    /** @note Doesn't alter the component's attribute. */
    void ToggleVisibility();

    /// Return whether is attached to the Ogre scene node hierarchy
    bool IsAttached() const { return attached_; }

signals:
    /// emitted when position has changed.
    void PositionChanged(const QVector3D &pos);

    /// emitted when rotation has changed.
    void OrientationChanged(const QQuaternion &rot);

    /// emitted when scale has changed.
    void ScaleChanged(const QVector3D &scale);

    /// Emitted when about to be destroyed
    void AboutToBeDestroyed();

private slots:
    /// Handle attributechange
    /** @param attribute Attribute that changed.
        @param change Change type.
     */
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
};
