// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Sound_EC_Sound_h
#define incl_EC_Sound_EC_Sound_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "AssetReference.h"
#include "AssetFwd.h"
#include "AudioFwd.h"

// Undef PlaySound from WIN32 API
#ifdef PlaySound
#undef PlaySound
#endif

/// Represents in-world sound source.
/**
<table class="header">
<tr>
<td>
<h2>Sound</h2>
Represents in-world sound source. The audio clip to play is specified in the soundRef attribute. Setting
the soundRef attribute does not immediately trigger sound playback. Use the "PlaySound" action to do it.

@note If the spatial attribute is true, and th entity this EC_Sound is part of contains an EC_Placeable 
    component, this sound clip is treated as a spatial (3D) sound. Otherwise, the sound is treated as a
    nonpositional (ambient) sound.

@note If sound attributes has been changed while the audio clip is on playing state, user needs to call
UpdateSoundSettings() to apply those changes into the Audio API.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>AssetReference: soundRef
<div>Sound asset reference that is used to request a sound from sound service.</div> 
<li>float: soundInnerRadius
<div>Sound inner radius tell the distance where sound gain value is in it's maximum.</div> 
<li>float: soundOuterRadius
<div>Sound outer radius tell the distance where sound gain value is zero.</div> 
<li>float: soundGain
<div>Sound gain value should be between 0.0-1.0</div> 
<li>bool: loopSound
<div>Do we want to loop the sound until the stop sound is called.</div> 
<li>bool: spatial
<div>If true, the audio source is played back as a spatial (3D) sound source. This requires a 
EC_Placeable component to be present in the same entity as this EC_Sound component. Otherwise,
the sound is played back as a nonspatial audio clip.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"PlaySound": Starts playing the sound.
<li>"StopSound": Stops playing the sound.
<li>"UpdateSoundSettings": Get each attribute value and send them over to sound service.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>"PlaySound": Starts playing the sound.
<li>"StopSound": Stops playing the sound.
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable</b>.
</table>
*/
class EC_Sound : public IComponent
{
    DECLARE_EC(EC_Sound);
    Q_OBJECT

public:
    ~EC_Sound();
    virtual bool IsSerializable() const { return true; }

    //! Sound asset reference.
    Q_PROPERTY(AssetReference soundRef READ getsoundRef WRITE setsoundRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, soundRef);

    //! Sound inner radius tell the distance where sound gain value is in it's maximum.
    Q_PROPERTY(float soundInnerRadius READ getsoundInnerRadius WRITE setsoundInnerRadius);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundInnerRadius);

    //! Sound outer radius tell the distance where sound gain value is zero.
    Q_PROPERTY(float soundOuterRadius READ getsoundOuterRadius WRITE setsoundOuterRadius);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundOuterRadius);

    //! Sound gain value should be between 0.0-1.0
    Q_PROPERTY(float soundGain READ getsoundGain WRITE setsoundGain);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundGain);

    //! Do we want to play this sound when the asset is loaded.
    Q_PROPERTY(bool playOnLoad READ getplayOnLoad WRITE setplayOnLoad);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, playOnLoad);

    //! Do we want to loop the sound until the stop sound is called.
    Q_PROPERTY(bool loopSound READ getloopSound WRITE setloopSound);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, loopSound);

    //! Is the sound type spatial or ambient
    Q_PROPERTY(bool spatial READ getspatial WRITE setspatial);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, spatial);

public slots:
    //! Show the asset in asset viewer window.
    ///\todo implement
    void View(const QString &attributeName);

    //! Starts playing the sound.
    void PlaySound();

    //! Stops playing the sound.
    void StopSound();

    //! Get each attribute value and send them over to sound service.
    void UpdateSoundSettings();

    /// Finds from the current scene the SoundListener that is currently active, or null if no SoundListener is active.
    Scene::EntityPtr GetActiveSoundListener();

private slots:
    //! Update signals to parents signals.
    void UpdateSignals();

    //! Monitors own attribute changes.
    void AttributeUpdated(IAttribute *attribute);

    //! Handles asset loaded signal.
    void AudioAssetLoaded(AssetPtr asset);

    //! Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

    //! Update the EC_Sound position to match placeable position.
    void PositionChange(const QVector3D &pos);

private:
    explicit EC_Sound(IModule *module);
    ComponentPtr FindPlaceable() const;
    SoundChannelPtr soundChannel;
};

#endif
