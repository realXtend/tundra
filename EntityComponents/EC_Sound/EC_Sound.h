// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Sound_EC_Sound_h
#define incl_EC_Sound_EC_Sound_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"

/// Represents in-world sound source.
/**
<table class="header">
<tr>
<td>
<h2>Sound</h2>
Represents in-world sound source.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>QString: soundId
<div>Sound asset reference that is used to request a sound from sound service.</div> 
<li>float: soundInnerRadius
<div>Sound inner radius tell the distance where sound gain value is in it's maximum.</div> 
<li>float: soundOuterRadius
<div>Sound outer radius tell the distance where sound gain value is zero.</div> 
<li>float: soundGain
<div>Sound gain value should be between 0.0-1.0</div> 
<li>bool: loopSound
<div>Do we want to loop the sound until the stop sound is called.</div> 
<li>bool: triggerSound
<div>Set this attribute true when you want to trigger the sound.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"PlaySound": 
<li>"StopSound":
<li>"UpdateSoundSettings": Get each attribute value and send them over to sound service.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>"PlaySound": 
<li>"StopSound":
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgrePlaceable</b>.
</table>
*/
class EC_Sound : public IComponent
{
    DECLARE_EC(EC_Sound);
    Q_OBJECT
public:
    ~EC_Sound();
    virtual bool IsSerializable() const { return true; }

    Q_PROPERTY(QString soundId READ getsoundId WRITE setsoundId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, soundId);

    Q_PROPERTY(float soundInnerRadius READ getsoundInnerRadius WRITE setsoundInnerRadius);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundInnerRadius);

    Q_PROPERTY(float soundOuterRadius READ getsoundOuterRadius WRITE setsoundOuterRadius);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundOuterRadius);

    Q_PROPERTY(float soundGain READ getsoundGain WRITE setsoundGain);
    DEFINE_QPROPERTY_ATTRIBUTE(float, soundGain);

    Q_PROPERTY(bool loopSound READ getloopSound WRITE setloopSound);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, loopSound);

    Q_PROPERTY(bool triggerSound READ gettriggerSound WRITE settriggerSound);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, triggerSound);

public slots:
    void PlaySound();
    void StopSound();
    //! Get each attribute value and send them over to sound service.
    void UpdateSoundSettings();

private slots:
    void UpdateSignals();
    void AttributeUpdated(IAttribute *attribute);
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

private:
    explicit EC_Sound(IModule *module);
    ComponentPtr FindPlaceable() const;

    sound_id_t sound_id_;
};

#endif
