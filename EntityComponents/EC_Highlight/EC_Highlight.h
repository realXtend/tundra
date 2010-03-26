/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Highlight.h
 *  @brief  EC_Highlight enables visual highlighting effect for of scene entity.
 *  @note   The entity must have EC_OgrePlaceable and EC_OgreMesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#ifndef incl_EC_Highlight_EC_Highlight_h
#define incl_EC_Highlight_EC_Highlight_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <QStringList>
#include <QFont>
#include <QColor>

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class Entity;
}

class EC_Highlight : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_Highlight);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Highlight(Foundation::ModuleInterface *module);

public:
    /// Destructor.
    ~EC_Highlight();

public slots:
    /// Shows the highlighting effect.
    void Show();

    /// Hides the highlighting effect.
    void Hide();

    /// Returns if the highlight component is visible or not.
    /// @true If the highlight component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

private:
    /// Creates the clone entity used for highlighting from the original.
    void Create();

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre entity clone created for highlighting.
    Ogre::Entity *entityClone_;

    /// Ogre scene node where this EC is attached.
    Ogre::SceneNode *sceneNode_;

    /// Name of the cloned entity used for highlighting
    std::string cloneName_;
};

#endif
