/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_SoundListener.h
    @brief  Entity-component which provides sound listener position for in-world 3D audio. */

#pragma once

#include "IComponent.h"

class EC_Placeable;

/// Entity-component which provides sound listener position for in-world 3D audio.
/** <table class="header">
    <tr>
    <td>
    <h2>SoundListener</h2>
    Entity-component which provides sound listener position for in-world 3D audio.
    Updates parent entity's placeable component's position to the sound system each frame.

    @note   Only one entity can have active sound listener at a time.

    <b>Attributes</b>.
    <ul>
    <li>bool: active
    <div> @copydoc active </div>
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

    <b>Depends on @ref EC_Placeable "Placeable".</b>
    </table> */
class EC_SoundListener : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_SoundListener", 7)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_SoundListener(Scene* scene);
    /// Detaches placeable component from this entity.
    ~EC_SoundListener();

    /// Is this listener active or not
    Q_PROPERTY(bool active READ getactive WRITE setactive);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, active);

private:
    /// Parent entity's placeable component.
    weak_ptr<EC_Placeable> placeable_;
    
private slots:
    /// Called when component changes.
    /** If this listener component is set active it iterates the scene and
     disables all the other sound listeners.
     */
    void AttributesChanged();

    /// Retrieves placeable component when parent entity is set.
    void RetrievePlaceable();

    /// Updates listeners position for sound system, is this listener is active. Called each frame.
    void Update();

    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();
};
