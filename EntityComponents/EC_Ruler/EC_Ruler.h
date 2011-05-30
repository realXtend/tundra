/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Ruler.cpp
 *  @brief  EC_Ruler provides visualisation for transformations.
 *  @note   The entity must have EC_Placeable component available in advance.
 *
 */

#pragma once

#include "IComponent.h"
#include "Declare_EC.h"

#include <QVector3D>
#include <QQuaternion>

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class ManualObject;
}
/**
<table class="header">
<tr>
<td>
<h2>Ruler</h2>
Ruler provides visualisation for transformations.

Registered by PythonScriptModule.

<b>Attributes</b>:
<ul>
<li>int: Type
<div>Ruler type</div> 
<li>bool: visible
<div>Visibility.</div> 
<li>int: axis
<div>Axis.</div> 
<li>bool: local
<div>Local or global space.</div> 
<li>float: radius
<div></div> 
<li>float: segments
<div></div> 
</ul>


<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows the highlighting effect.
<li>"Hide": Hides the highlighting effect.
<li>"IsVisible": 
<li>"SetType": set the ruler type to show: 0 = translate, 1 = rotate, 2 = scale
<li>"StartDrag": Call StartDrag to initialise some values used to update the selected ruler.
<li>"EndDrag": Call EndDrag to tell the code we're done for now.
<li>"UpdateRuler": Callback for OnChanged from ECEditor. 
</ul>
     

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>The entity must have EC_Placeable and EC_Mesh (if mesh) or
EC_OgreCustomObject (if prim) components available in advance</b>.
</table>

*/
class EC_Ruler : public IComponent
{
    Q_OBJECT
    Q_ENUMS(Type)
    DECLARE_EC(EC_Ruler);
    

public:
    /// Destructor.
    ~EC_Ruler();
    
    enum Type
    {
        Null,
        Rotation,
        Scale,
        Translation,
    };
    
    enum Axis
    {
        Y,
        X,
        Z
    };
    
    /// Ruler type
    Attribute<int> typeAttr_;
    
    /// Visibility
    Attribute<bool> visibleAttr_;
    
    /// Axis
    Attribute<int> axisAttr_;
    
    /// Local or global space
    Attribute<bool> localAttr_;
    
    Attribute<float> radiusAttr_;
    
    Attribute<float> segmentsAttr_;

public slots:

    /// Returns if the ruler component is visible or not.
    /// @true If the rule component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;
    
    void SetVisible(bool visible) { visibleAttr_.Set(visible, AttributeChange::Default); }
    
    void SetRadius(float radius) { radiusAttr_.Set(radius, AttributeChange::Default); }
    
    /// set the ruler type to show: 0 = translate, 1 = rotate, 2 = scale
    void SetType(EC_Ruler::Type type) { typeAttr_.Set(type, AttributeChange::Default); }
    
    void SetAxis(int axis) { axisAttr_.Set(axis, AttributeChange::Default); }
    
    /// Call StartDrag to initialise some values used to update the selected ruler
    void StartDrag(QVector3D pos, QQuaternion rot, QVector3D scale);
    
    /// Handle drag event
    void DoDrag(QVector3D pos, QQuaternion rot, QVector3D scale);
    
    /// Call EndDrag to tell the code we're done for now
    void EndDrag();
    
    /// Callback for OnChanged from ECEditor
    void UpdateRuler();

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Ruler(IModule *module);

    /// Creates the ruler objects.
    void Create();
    
    void SetupRotationRuler();
    void SetupTranslateRuler();
    void SetupScaleRuler();

    /// Shows the ruler.
    void Show();

    /// Hides the ruler.
    void Hide();
    
    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre entity clone created for highlighting.
    Ogre::ManualObject *rulerObject;
    /// object for grid part.
    Ogre::ManualObject *gridObject;

    /// Ogre scene node where this EC is attached.
    Ogre::SceneNode *sceneNode_;
    
    /// Ogre scene node to attach EC to when we want global space axis vis
    Ogre::SceneNode *globalSceneNode;
    /// grid anchor
    Ogre::SceneNode *anchorNode;
 
    
    /// Name of the ruler static part
    std::string rulerName;
    /// Name of the ruler dynamic part
    std::string rulerMovingPartName;
    /// Name of the static scene node
    std::string nodeName;
    /// Name of the dynamic scene node
    std::string movingNodeName;
    
    /// Type of the ruler currently shown
    EC_Ruler::Type type;
    
    /// Start position of entity to which this EC is attached
    QVector3D pos_;
    /// Start scale of entity to which this EC is attached
    QVector3D scale_;
    /// Start orientation of entity to which this EC is attached
    QQuaternion rot_;
    
    /// New position of entity to which this EC is attached
    QVector3D newpos_;
    /// New scale of entity to which this EC is attached
    QVector3D newscale_;
    /// New orientation of entity to which this EC is attached
    QQuaternion newrot_;
};

