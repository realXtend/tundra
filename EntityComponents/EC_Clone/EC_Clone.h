/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Clone.h
 *  @brief  EC_Clone creates an OGRE clone entity from the the original entity.
 *          This component can be used e.g. when visualizing object duplication in the world.
 *  @note   The entity must have EC_Placeable and EC_Mesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#ifndef incl_EC_Clone_EC_Clone_h
#define incl_EC_Clone_EC_Clone_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <QStringList>
#include <QFont>
#include <QColor>
#include <QtGui/qvector3d.h>

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class Entity;
}

/**
<table class="header">
<tr>
<td>
<h2>Clone</h2>
Clone creates an OGRE clone entity from the the original entity.
This component can be used e.g. when visualizing object duplication in the world.

Registered by RexLogic::RexLogicModule.

<b>No Attributes.</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows the clone  
<li>"Hide": Hide the clone
<li>"IsVisible": Returns if the clone visualization component is visible or not.
    @true If the clone visualization component is visible, false if it's hidden or not initialized properly.
<li>"GetPosition": Returns the position of the clone component.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

<b>The entity must have EC_Placeable and EC_Mesh (if mesh) or
EC_OgreCustomObject (if prim) components available in advance.</b>
</table>
*/


class EC_Clone : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Clone);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Clone(IModule *module);

public:
    /// Destructor.
    ~EC_Clone();

public slots:
    /// Shows the clone.
    void Show();

    /// Hides the clone.
    void Hide();

    /// Returns if the clone visualization component is visible or not.
    /// @true If the clone visualization component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

    /// Returns the position of the clone component.
    QVector3D GetPosition() const;

private:
    /// Creates the clone.
    void Create();

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre entity clone created for visualization.
    Ogre::Entity *entityClone_;

    /// Ogre scene node where this EC is attached.
    Ogre::SceneNode *sceneNode_;

    /// Name of the cloned entity.
    std::string cloneName_;
};

#endif
