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

#include "ComponentInterface.h"
#include "AttributeInterface.h"
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

class EC_Touchable : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_Touchable);

public:
    /// Destructor.
    ~EC_Touchable();

    /// ComponentInterface override.
    bool IsSerializable() const { return true; }

    /// Name of the material used for this EC.
    Foundation::Attribute<std::string> materialName;

    /// Time the touchable effect is visible in seconds.
    Foundation::Attribute<float> visibilityTime;

    /// Called by rexlogic when EVENT_ENTITY_MOUSE_HOVER upon this
    void OnHover();

    /// Called by rexlogic when EVENT_ENTITY_CLICKED on this
    void OnClick();


public slots:
    /// Shows the effect.
    void Show();

    /// Hides the effect.
    void Hide();

    /// Returns if the component is visible or not.
    /// @true If the component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

private slots:
    /// Updates the component if its material changes.
    void UpdateMaterial();

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Touchable(Foundation::ModuleInterface *module);

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

signals:
    //! Signal when mouse hovers over the entity with this touchable component
    void MouseHover(); //\todo change RaycastResult to a QObject with QVector3D etc and put here
    void Clicked(); //consider naming, qt has .clicked .. browser .onclick(?)
};

#endif
