/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Ruler.h
 *  @brief  EC_Ruler enables visual highlighting effect for of scene entity.
 *  @note   The entity must have EC_Placeable and EC_Mesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#ifndef incl_EC_Ruler_EC_Ruler_h
#define incl_EC_Ruler_EC_Ruler_h

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
Ruler enables visual highlighting effect for of scene entity.


Registered by RexLogic::RexLogicModule.

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
    
    //! Ruler type
    Attribute<int> typeAttr_;
    
    //! Visibility
    Attribute<bool> visibleAttr_;
    
    //! Axis
    Attribute<int> axisAttr_;
    
    //! Local or global space
    Attribute<bool> localAttr_;
    
    Attribute<float> radiusAttr_;
    
    Attribute<float> segmentsAttr_;

public slots:

    /// Returns if the ruler component is visible or not.
    /// @true If the rule component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;
    
    void SetVisible(bool visible) { visibleAttr_.Set(visible, AttributeChange::Default); }
    
    void SetRadius(float radius) { radiusAttr_.Set(radius, AttributeChange::Default); }
    
    //! set the ruler type to show: 0 = translate, 1 = rotate, 2 = scale
    void SetType(EC_Ruler::Type type) { typeAttr_.Set(type, AttributeChange::Default); }
    
    void SetAxis(int axis) { axisAttr_.Set(axis, AttributeChange::Default); }
    
    //! Call StartDrag to initialise some values used to update the selected ruler
    void StartDrag(QVector3D pos, QQuaternion rot, QVector3D scale);
    
    void DoDrag(QVector3D pos, QQuaternion rot, QVector3D scale);
    
    //! Call EndDrag to tell the code we're done for now
    void EndDrag();
    
    //! Callback for OnChanged from ECEditor
    void UpdateRuler();

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Ruler(IModule *module);

    /// Creates the clone entity used for highlighting from the original.
    void Create();
    
    void SetupRotationRuler();
    void SetupTranslateRuler();
    void SetupScaleRuler();

    /// Shows the highlighting effect.
    void Show();

    /// Hides the highlighting effect.
    void Hide();
    
    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre entity clone created for highlighting.
    Ogre::ManualObject *rulerObject;

    /// Ogre scene node where this EC is attached.
    Ogre::SceneNode *sceneNode_;
    
    /// Ogre scene node to attach EC to when we want global space axis vis
    Ogre::SceneNode *globalSceneNode;
    
    std::string rulerName;
    std::string nodeName;
    
    EC_Ruler::Type type;
    
    QVector3D pos_;
    QVector3D scale_;
    QQuaternion rot_;
    
    QVector3D newpos_;
    QVector3D newscale_;
    QQuaternion newrot_;
};

#endif
