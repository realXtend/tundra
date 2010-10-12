/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SoundListener.h
 *  @brief  Entity-component which provides sound listener position for in-world 3D audio.
 *          Updates parent entity's placeable component's position to the sound service each frame.
 *  @note   Only one entity can have active sound listener at a time.
 */

#ifndef incl_EC_SoundListener_EC_SoundListener_h
#define incl_EC_SoundListener_EC_SoundListener_h

#include "IComponent.h"
#include "Declare_EC.h"


class ISoundService;
class EC_Placeable;

/// Entity-component which provides sound listener position for in-world 3D audio.
/**
<table class="header">
<tr>
<td>
<h2>SoundListener</h2>
Entity-component which provides sound listener position for in-world 3D audio.
Updates parent entity's placeable component's position to the sound service each frame.
@note   Only one entity can have active sound listener at a time.
@todo   EC_SoundListener is currently only working with avatar and freecamera. In future, there
        should be an option to apply a sound listener component to any entity that owns a
        instance of placeable component.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>.
<ul>
<li>bool: active
<div>Is this listener active or not.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>"Active": Make this sound listener active and put other listeners to inactive state.
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on Placeable.</b>
</table>
*/
class EC_SoundListener : public IComponent
{
    DECLARE_EC(EC_SoundListener);
    Q_OBJECT

public:
    /// Destructor. Detaches placeable component from this entity.
    ~EC_SoundListener();

    Q_PROPERTY(bool active READ getactive WRITE setactive);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, active);

private slots:
    /// Retrieves placeable component when parent entity is set.
    void RetrievePlaceable();

    /// Updates listeners position for sound service, is this listener is active. Called each frame.
    void Update();

private:
    /// Constructor.
    /** @param module Declaring module.
    */
    explicit EC_SoundListener(IModule *module);

    /// Parent entity's placeable component.
    boost::weak_ptr<EC_Placeable> placeable_;

    /// Sound service.
    boost::weak_ptr<ISoundService> soundService_;

private slots:
    /// Called when component changes.
    /** If this listener component is set active it iterates the scene and
        disables all the other sound listeners.
    */
    void OnActiveChanged();

    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();
};

#endif
