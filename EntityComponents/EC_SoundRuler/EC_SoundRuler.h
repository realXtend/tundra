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

#include "IComponent.h"
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

/**

<table class="header">
<tr>
<td>
<h2>SoundRuler</h2>
SoundRuler enables visual sound properties effect for scene entity.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>float: radiusAttr_
<div>Sound radius</div> 
<li>float: volumeAttr_
<div>Sound volume</div> 
<li>float: segmentsAttr_
<div> Segments to use for radius circle</div> 
</ul>


<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows the highlighting effect. 
<li>"Hide": Hides the highlighting effect.
<li>"SetVolume": Set volume to show
<li>"SetRadius": Set radius to show
<li>"IsVisible": Returns if the ruler component is visible or not.
		@true If the rule component is visible, false if it's hidden or not initialized properly.
<li>"UpdateSoundRuler": Callback for OnChanged from ECEditor
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on OgrePlaceable, OgreMesh, OgreCustomObject and OpenSimPrim</b>. @author Nathan Letwory | http://www.letworyinteractive.com
</table>

*/
class EC_SoundRuler : public IComponent
{


    Q_OBJECT
    DECLARE_EC(EC_SoundRuler);
    

public:
    /// Destructor.
    ~EC_SoundRuler();
    
    //! Sound radius
    Attribute<float> radiusAttr_;
    
    //! Sound volume
    Attribute<float> volumeAttr_;
    
    //! Segments to use for radius circle
    Attribute<float> segmentsAttr_;

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
    explicit EC_SoundRuler(IModule *module);

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
    
    // Name for this ruler
    std::string rulerName;
    
    // Name for container node
    std::string nodeName;
};

#endif
