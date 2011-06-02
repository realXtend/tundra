// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_Placeable_h
#define incl_OgreRenderer_EC_Placeable_h

#include "IComponent.h"
#include "IAttribute.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Transform.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "Declare_EC.h"

#include <QQuaternion>
#include <QVector3D>

//! Ogre placeable (scene node) component
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
<div>Sets the position, rotation and scale of the entity. Not usable and not replicated in Opensim worlds.</div>
<li>bool: drawDebug
<div>Shows the debug bounding box of geometry attached to the placeable.</div>
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
    DECLARE_EC(EC_Placeable);

    Q_OBJECT
    
public:
    //! position property
    //! @note This affects internally the transform attribute, but allows QVector3D access to it
    Q_PROPERTY(QVector3D position READ GetQPosition WRITE SetQPosition)
    
    //! scale property
    //! @note This affects internally the transform attribute, but allows QVector3D access to it
    Q_PROPERTY(QVector3D scale READ GetQScale WRITE SetQScale)

    //! orientation property
    //! @note This affects internally the transform attribute, but allows QQuaternion access to it
    Q_PROPERTY(QQuaternion orientation READ GetQOrientation WRITE SetQOrientation)

    //! orientationEuler property
    //! @note This affects internally the transform attribute, but allows QVector3D euler angle (in degrees) access to it
    Q_PROPERTY(QVector3D orientationEuler READ GetQOrientationEuler WRITE SetQOrientationEuler)
    
    Q_PROPERTY(QVector3D LocalXAxis READ GetQLocalXAxis)
    Q_PROPERTY(QVector3D LocalYAxis READ GetQLocalYAxis)
    Q_PROPERTY(QVector3D LocalZAxis READ GetQLocalZAxis)
    Q_PROPERTY(float Yaw READ GetYaw)
    Q_PROPERTY(float Pitch READ GetPitch)
    Q_PROPERTY(float Roll READ GetRoll)
    Q_PROPERTY(int SelectPriority READ GetSelectPriority WRITE SetSelectPriority)

    //! Transformation attribute for position, rotation and scale adjustments.
    Q_PROPERTY(Transform transform READ gettransform WRITE settransform);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, transform);
    
    //! Show debug bounding box -attribute
    Q_PROPERTY(bool drawDebug READ getdrawDebug WRITE setdrawDebug);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, drawDebug);

    //! Specifies whether any objects attached to the scene node of this placeable are visible or not.
    Q_PROPERTY(bool visible READ getvisible WRITE setvisible);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, visible);

    virtual ~EC_Placeable();
    
    //! Set component as serializable.
    /*! Note that despite this, in OpenSim worlds, the network sync will be disabled from the component,
        as EC_NetworkPosition controls the actual authoritative position (including interpolation)
     */
    virtual bool IsSerializable() const { return true; }
    
    //! orients to look at a point in space
    /*! \param look_at point to look at
     */
    void LookAt(const Vector3df& look_at);
    
    //! sets select priority
    /*! \param priority new select priority
     */
    void SetSelectPriority(int priority) { select_priority_ = priority; }
       
    //! returns position
    Vector3df GetPosition() const;
    //! returns orientation
    Quaternion GetOrientation() const;
    //! returns scale
    Vector3df GetScale() const;

    //! Get the local X axis from the node orientation
    Vector3df GetLocalXAxis() const;
    //! Get the local Y axis from the node orientation
    Vector3df GetLocalYAxis() const;
    //! Get the local Z axis from the node orientation
    Vector3df GetLocalZAxis() const;
    //! Get the local X axis from the node orientation
    QVector3D GetQLocalXAxis() const;
    //! Get the local Y axis from the node orientation
    QVector3D GetQLocalYAxis() const;
    //! Get the local Z axis from the node orientation
    QVector3D GetQLocalZAxis() const;
    
    //! returns Ogre scenenode for attaching geometry.
    /*! Do not manipulate the pos/orientation/scale of this node directly
     */
    Ogre::SceneNode* GetSceneNode() const { return scene_node_; }
   
    //! returns Ogre scenenode for linking another placeable in OpenSim compatible way.
    /*! Only used by other placeables, or in other rare cases. Do not use if GetSceneNode() works for you,
        as this doesn't take scaling into account!
     */
    Ogre::SceneNode* GetLinkSceneNode() const { return link_scene_node_; }
    
    //! returns select priority
    int GetSelectPriority() const { return select_priority_; }

    //! experimental accessors that use the new 3d vector etc types in Qt 4.6, for qproperties
    QVector3D GetQPosition() const;
    void SetQPosition(QVector3D newpos);

    //! get node orientation
    QQuaternion GetQOrientation() const;
    //! set node orientation
    void SetQOrientation(QQuaternion newrot);

    //! get node orientation as euler (degrees)
    QVector3D GetQOrientationEuler() const;
    //! set node orientation as euler (degrees)
    void SetQOrientationEuler(QVector3D newrot);

    //! set node scale
    QVector3D GetQScale() const;
    //! get node scale
    void SetQScale(QVector3D newscale);


public slots:
    //! sets position
    /*! \param position new position
     */
    void SetPosition(const Vector3df& position);
    //! sets orientation
    /*! \param orientation new orientation
     */
    void SetOrientation(const Quaternion& orientation);
    //! sets scale
    /*! \param scale new scale
     */
    void SetScale(const Vector3df& scale);

    //! gets parent placeable
    ComponentPtr GetParent() { return parent_; }

    //! sets parent placeable
    /*! set null placeable to attach to scene root (the default)
        \param placeable new parent
     */
    void SetParent(ComponentPtr placeable);
    
    //! translate
    /* 0 = x, 1 = y, 2 = z. returns new position */
    QVector3D translate( int axis, float amount);

    //! LookAt wrapper that accepts a QVector3D for py & js e.g. camera use
    void LookAt(const QVector3D look_at) { LookAt(Vector3df(look_at.x(), look_at.y(), look_at.z())); }
    
    //! Translates the placeable. Modifies the transform attribute using Default changetype.
    void Translate(const Vector3df& translation);
    
    //! Translates the placeable relative to orientation. Modifies the transform attribute using Default changetype.
    void TranslateRelative(const Vector3df& translation);
    
    //! Transforms a vector by the placeable's orientation
    Vector3df GetRelativeVector(const Vector3df& vec);
    
    //! Calculates rotation
    Vector3df GetRotationFromTo(const Vector3df& from, const Vector3df& to);

    Vector3df GetDerivedPosition(); 
    Quaternion GetDerivedOrientation();

    //! yaws the node
    /*! \param radians how many radians to yaw
     */
    void SetYaw(float radians);
    //! pitches the node
    /*! \param radians how many radians to pitch
     */
    void SetPitch(float radians);
    //! rolls the node
    /*! \param radians how many radians to roll
     */
    void SetRoll(float radians);
    //! pitches the node
    /*! \param radians how many radians to pitch
     */
    void SetPitchWorld(float radians);
    //! rolls the node
    /*! \param radians how many radians to roll
     */
    void SetRollWorld(float radians);

    //! get the yaw of the node
    float GetYaw() const;
    //! get the pitch of the node
    float GetPitch() const;
    //! get the roll of the node
    float GetRoll() const;

    /// Shows the Entity
    void Show();

    /// Hides the Entity
    void Hide();

    /// Toggle Visibility
    void ToggleVisibility();

signals:
    //! emmitted when position has changed.
    void PositionChanged(const QVector3D &pos);

    //! emmitted when rotation has changed.
    void OrientationChanged(const QQuaternion &rot);

    //! emmitted when scale has changed.
    void ScaleChanged(const QVector3D &scale);

private slots:
    //! Handle attributechange
    /*! \param attribute Attribute that changed.
        \param change Change type.
     */
    void HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Registers the action this EC provides to the parent entity, when it's set.
	void RegisterActions();

private:
    //! constructor
    /*! \param module renderer module
     */
    explicit EC_Placeable(IModule* module);
    
    //! attaches scenenode to parent
    void AttachNode();
    
    //! detaches scenenode from parent
    void DetachNode();
    
    //! renderer
    OgreRenderer::RendererWeakPtr renderer_;
    
    //! parent placeable
    ComponentPtr parent_;
    
    //! Ogre scene node for geometry. scale is handled here
    Ogre::SceneNode* scene_node_;

    //! Ogre scene node for linking. position & orientation are handled here, and the geometry scene node is attached to this
    Ogre::SceneNode* link_scene_node_;
    
    //! attached to scene hierarchy-flag
    bool attached_;
    
    //! selection priority for picking
    int select_priority_;
};

#endif
