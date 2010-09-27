/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Touchable.h
 *  @brief  EC_Touchable enables visual effect for scene entity in cases where
 *          the entity can be considered to have some kind of functionality
 *          when it is clicked i.e. touched. The effect is not visible by default.
 *          You must call Show() function separately. The effect is visible only
 *          for certain time.
 *  @note   The entity must have EC_OgrePlaceable and EC_OgreMesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#ifndef incl_EC_Highlight_EC_Touchable_h
#define incl_EC_Highlight_EC_Touchable_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class Entity;
}

/// Touchable enables visual effect for scene entity in cases where the entity can be 
/// considered to have some kind of functionality when it is clicked i.e. touched.
/**
<table class="header">
<tr>
<td>
<h2>Touchable</h2>
Touchable enables visual effect for scene entity in cases where
the entity can be considered to have some kind of functionality
when it is clicked i.e. touched. The effect is not visible by default.
You must call Show() function separately. The effect is visible only
for certain time.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>QString : materialName
<div>Name of the material used for this EC. </div>
</ul>
<ul>
<li>bool : highlightOnHover
<div>Is highlight material shown on mouse hover.</div>
</ul>
<ul>
<li>int : hoverCursor
<div>Hover mouse cursor, see @see Qt::CursorShape</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows the effect.
<li>"Hide": Hides the effect.
<li>"IsVisible": Returns if the component is visible or not.
        @true If the component is visible, false if it's hidden or not initialized properly.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>"Show": Shows the effect.
<li>"Hide": Hides the effect.
<li>"MouseHoverIn": 
<li>"MouseHover": 
<li>"MouseHoverOut": 
<li>"MousePressed": 
</ul>
</td>
</tr>

Does not emit any actions.

<b>The entity must have EC_OgrePlaceable and EC_OgreMesh (if mesh) or
EC_OgreCustomObject (if prim) components available in advance.</b>. 
</table>
*/
class EC_Touchable : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Touchable);

public:
    /// Destructor.
    ~EC_Touchable();

    /// IComponent override.
    bool IsSerializable() const { return true; }

    /// Name of the material used for this EC.
    Attribute<QString> materialName;

    /// Is highlight material shown on mouse hover.
    Attribute<bool> highlightOnHover;

    /// Hover mouse cursor, see @see Qt::CursorShape
    Attribute<int> hoverCursor;

public slots:
    /// Shows the effect.
    void Show();

    /// Hides the effect.
    void Hide();

    /// Returns if the component is visible or not.
    /// @true If the component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

signals:
    //! Signal when mouse hovers over the entity with this touchable component
    void MouseHover(); //\todo change RaycastResult to a QObject with QVector3D etc and put here
    void MouseHoverIn();
    void MouseHoverOut();
    void MousePressed();

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Touchable(IModule *module);

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre entity clone created for highlighting.
    Ogre::Entity *entityClone_;

    /// Ogre scene node where this EC is attached.
    Ogre::SceneNode *sceneNode_;

    /// Name of the cloned entity used for highlighting
    std::string cloneName_;

private slots:
    /// Creates the clone entity used for highlighting from the original.
    void Create();

    /// Updates the component if its material changes.
    void UpdateMaterial();

    /// Set our hover cursor visible
    void SetCursorVisible(bool visible);

    /// Slot for emitting HoverIn the first time we get OnHover() call
    void OnHoverIn();

    /// Shows the effect and sets override cursor.
    void OnHover();

    /// Hides the effect and restores override cursor.
    void OnHoverOut();

    /// \todo Remove this altogether
    void OnClick();

    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();
};

#endif
