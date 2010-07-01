/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Ruler.h
 *  @brief  EC_Ruler enables visual highlighting effect for of scene entity.
 *  @note   The entity must have EC_OgrePlaceable and EC_OgreMesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#ifndef incl_EC_Ruler_EC_Ruler_h
#define incl_EC_Ruler_EC_Ruler_h

#include "ComponentInterface.h"
#include "Declare_EC.h"

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class ManualObject;
}

class EC_Ruler : public Foundation::ComponentInterface
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
        X,
        Y,
        Z
    };
    
    //! Ruler type
    Foundation::Attribute<int> typeAttr_;
    
    //! Visibility
    Foundation::Attribute<bool> visibleAttr_;
    
    //! Axis
    Foundation::Attribute<int> axisAttr_;

public slots:
    /// Shows the highlighting effect.
    void Show();

    /// Hides the highlighting effect.
    void Hide();

    /// Returns if the ruler component is visible or not.
    /// @true If the rule component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;
    
    //! set the ruler type to show: 0 = translate, 1 = rotate, 2 = scale
    void SetType(EC_Ruler::Type type);
    
    //! Call StartDrag to initialise some values used to update the selected ruler
    void StartDrag();
    
    //! Call EndDrag to tell the code we're done for now
    void EndDrag();
    
    //! Callback for OnChanged from ECEditor
    void UpdateRuler();

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_Ruler(Foundation::ModuleInterface *module);

    /// Creates the clone entity used for highlighting from the original.
    void Create();
    
    void SetupRotationRuler();
    void SetupTranslateRuler();

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre entity clone created for highlighting.
    Ogre::ManualObject *rulerObject;

    /// Ogre scene node where this EC is attached.
    Ogre::SceneNode *sceneNode_;
    
    std::string rulerName;
    
    EC_Ruler::Type type;
};

#endif
