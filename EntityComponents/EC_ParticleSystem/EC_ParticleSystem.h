// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_ParticleSystem_EC_ParticleSystem_h
#define incl_EC_ParticleSystem_EC_ParticleSystem_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "OgreModuleFwd.h"
#include "AssetReference.h"

/// Particle system.
/**
<table class="header">
<tr>
<td>
<h2>ParticleSystem</h2>

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>AssetReference: particleRef
<div>Particle resource asset reference.</div> 
<li>bool: castShadows
<div>does particles cast shadows (mostly useless).</div> 
<li>float: renderingDistance
<div>Particles rendering distance.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"CreateParticleSystem": Create a new particle system. System name will be same as component name.
<li>"DeleteParticleSystem": Delete excisting particle system.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable</b>.
</table>
*/
class EC_ParticleSystem : public IComponent
{
    DECLARE_EC(EC_ParticleSystem);
    Q_OBJECT

public:
    ~EC_ParticleSystem();

    virtual bool IsSerializable() const { return true; }

    /// Particle asset reference
    Q_PROPERTY(AssetReference particleRef READ getparticleRef WRITE setparticleRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, particleRef);

    /// Does particles cast shadows (mostly useless).
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);

    /// Particles rendering distance.
    Q_PROPERTY(float renderingDistance READ getrenderingDistance WRITE setrenderingDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, renderingDistance);

public slots:
    /// Open the particle asset in asset viewer.
    void View(const QString &attributeName);

    /// Create a new particle system. Does not delete the old
    void CreateParticleSystem(const QString &systemName);

    /// Delete all current particle systems.
    void DeleteParticleSystems();

private slots:
    void AttributeUpdated(IAttribute *attribute);
    void ParticleSystemAssetLoaded();
    void EntitySet();
    void OnComponentRemoved(IComponent *component, AttributeChange::Type change);
    
private:
    explicit EC_ParticleSystem(IModule *module);
    ComponentPtr FindPlaceable() const;

    OgreRenderer::RendererWeakPtr renderer_;
    std::vector<Ogre::ParticleSystem*> particleSystems_;
    Ogre::SceneNode* node_;
};

#endif
