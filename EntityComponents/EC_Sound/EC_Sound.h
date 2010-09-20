// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Sound_EC_Sound_h
#define incl_EC_Sound_EC_Sound_h

#include "IComponent.h"
#include "IAttribute.h"
#include "ResourceInterface.h"
#include "Declare_EC.h"

namespace Foundation
{
    class ModuleInterface;
}

/// Makes the sound component that is used to apply 3d sound sources to scene.
/**

<table style="margin: 20px;">
<tr>
<td style="width:500px; height: 100px; border: solid 1px black; background-color: #e0d0ff; vertical-align: top; padding: 5px;">
<h2>Mesh</h2>
Makes the sound component that is used to apply 3d sound sources to scene.

<b>Attributes</b>:
<ul>
<li>QString: Sound ref.
<div style="margin: 5px;">Sound asset reference that is used to request a sound from sound service.</div> 
<li>float: Sound radius inner. 
<div style="margin: 5px;">Sound inner radius tell the distance where sound gain value is in it's maximum.</div> 
<li>float: Sound radius outer.
<div style="margin: 5px;">Sound outer radius tell the distance where sound gain value is zero.</div> 
<li>bool: Loop sound.
<div style="margin: 5px;">Do we want to loop the sound until the stop sound is called.</div> 
<li>bool: Trigger sound.
<div style="margin: 5px;">Set this attribute troe when you want to trigger the sound.</div> 
<li>float: Sound gain.
<div style="margin: 5px;">Sound gain value should be between 0-1.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>void PlaySound();
<div style="margin: 5px;">Play sound.</div> 
<li>void StopSound();
<div style="margin: 5px;">Stop sound if active.</div> 
<li>void UpdateSoundSettings();
<div style="margin: 5px;">If sound is active and sound parameters has been changed this method will apply those changes to a sound service.</div>
</ul>
<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgrePlaceable</b>. The position in the OgrePlaceable component specifies the position in the world space where this sound is placed at. 
</table>

*/
class EC_Sound : public IComponent
{
    DECLARE_EC(EC_Sound);
    Q_OBJECT
public:
    ~EC_Sound();
    virtual bool IsSerializable() const { return true; }

    DEFINE_QPROPERTY_ATTRIBUTE(QString, soundId);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundInnerRadius);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundOuterRadius);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundGain);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, loopSound);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, triggerSound);

    /*Attribute<QString>      soundId_;
    Attribute<float>        soundInnerRadius_;
    Attribute<float>        soundOuterRadius_;
    Attribute<float>        soundGain_;
    Attribute<bool>         loopSound_;
    Attribute<bool>         triggerSound_;*/

public slots:
    void PlaySound();
    void StopSound();
    //! Get each attribute values and update sound's parameters if it's in playing state.
    void UpdateSoundSettings();

private slots:
    void UpdateSignals();
    void AttributeUpdated(IComponent *component, IAttribute *attribute);
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

private:
    explicit EC_Sound(IModule *module);
    ComponentPtr FindPlaceable() const;

    sound_id_t sound_id_;
};

#endif