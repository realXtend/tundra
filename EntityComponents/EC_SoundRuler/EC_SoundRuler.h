/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SoundRuler.h
 *  @brief  EC_SoundRuler enables visual sound properties effect for scene entity.
 *  @note   The entity must have EC_OgrePlaceable and EC_OpenSimPrim 
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */

#ifndef incl_EC_SoundRuler_EC_SoundRuler_h
#define incl_EC_SoundRuler_EC_SoundRuler_h

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

class EC_OpenSimPrim;

class EC_SoundRuler : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_SoundRuler);
    

public:
    /// Destructor.
    ~EC_SoundRuler();
    
    //! Visibility
    Foundation::Attribute<bool> visibleAttr_;
    
    //! Sound radius
    Foundation::Attribute<Real> radiusAttr_;
    
    //! Sound volume
    Foundation::Attribute<Real> volumeAttr_;
    
    //! Segments to use for radius circle
    Foundation::Attribute<Real> segmentsAttr_;

public slots:
    /// Shows the highlighting effect.
    void Show();

    /// Hides the highlighting effect.
    void Hide();
    
    /// Set volume to show
    void SetVolume(float volume) { volumeAttr_.Set(volume, AttributeChange::LocalOnly); }
    
    /// Set radius to show
    void SetRadius(float radius) { radiusAttr_.Set(radius, AttributeChange::LocalOnly); }

    /// Returns if the ruler component is visible or not.
    /// @true If the rule component is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;
    
    //! Callback for OnChanged from ECEditor
    void UpdateSoundRuler();

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_SoundRuler(Foundation::ModuleInterface *module);

    /// Creates the clone entity used for highlighting from the original.
    void Create();
    
    void SetupSoundRuler();

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre entity clone created for highlighting.
    Ogre::ManualObject *rulerObject;

    /// Ogre scene node where this EC is attached.
    Ogre::SceneNode *sceneNode_;
    
    /// Ogre scene node to attach EC to when we want global space axis vis
    Ogre::SceneNode *globalSceneNode;
    
    /// Holds prim
    EC_OpenSimPrim *prim;
};

#endif
