// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "IAttribute.h"
#include "OgreModuleFwd.h"
#include "AssetReference.h"
#include "AssetRefListener.h"
#include "OgreModuleApi.h"

/// Particle system.
/** <table class="header">
    <tr>
    <td>
    <h2>ParticleSystem</h2>

    <b>Attributes</b>:
    <ul>
    <li>AssetReference: particleRef
    <div> @copydoc particleRef </div>
    <li>bool: castShadows
    <div> @copydoc castShadows </div>
    <li>bool: enabled
    <div> @copydoc </div>
    <li>float: renderingDistance
    <div> @copydoc renderingDistance</div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"CreateParticleSystem": @copydoc CreateParticleSystem
    <li>"DeleteParticleSystem": @copydoc DeleteParticleSystem
    <li>"StopParticleSystem": @copydoc StopParticleSystem
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    <li>"StartParticleSystem": Starts a particle system. Usage: StartParticleSystem [systemName].
    If systemName is empty, all the particle systems in the particle asset will be started.
    <li>"HardStopParticleSystem": Hard-stops (deletes) a particle system. Usage: HardStopParticleSystem [systemName].
    If systemName is empty, all the particle systems in the particle asset will be deleted.
    <li>"SoftStopParticleSystem": Disables emitters on a particle system, but lets the existing particles run.
    Usage: SoftStopParticleSystem [systemName]. If systemName is empty, all the particle emitters in the particle asset will be stopped.
    </td>
    </tr>

    Does not emit any actions.

    <b>Depends on the component @ref EC_Placeable "Placeable".</b>
    </table> */
class OGRE_MODULE_API EC_ParticleSystem : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_ParticleSystem", 27)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_ParticleSystem(Scene* scene);

    ~EC_ParticleSystem();

    /// Particle asset reference
    Q_PROPERTY(AssetReference particleRef READ getparticleRef WRITE setparticleRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, particleRef);

    /// Does particles cast shadows.
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);

    /// Are the particle systems enabled.
    /** If true, will automatically create all particle systems from the asset.</div> */
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    /// Particles rendering distance.
    Q_PROPERTY(float renderingDistance READ getrenderingDistance WRITE setrenderingDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, renderingDistance);

public slots:
    /// Create a new particle system, or restart its emitters if stopped. Only one of each system in the particle asset can exist at a time. If system name is empty, create all systems from the asset.
    void CreateParticleSystem(const QString &systemName = QString());

    /// Delete a particle system by system name. If system name is empty, delete all systems.
    void DeleteParticleSystem(const QString& systemName = QString());

    /// Stop particle system emitters by system name (soft stop.) If system name is empty, stop all emitters.
    void SoftStopParticleSystem(const QString& systemName = QString());
    
private slots:
    void OnParticleAssetLoaded(AssetPtr asset);
    void OnParticleAssetFailed(IAssetTransfer* transfer, QString reason);
    void EntitySet();
    void OnComponentRemoved(IComponent *component, AttributeChange::Type change);

private:
    void AttributesChanged();
    ComponentPtr FindPlaceable() const;

    std::map<QString, Ogre::ParticleSystem*> particleSystems_;

    OgreWorldWeakPtr world_;

    /// Asset ref listener for the particle asset
    AssetRefListenerPtr particleAsset_;
};
