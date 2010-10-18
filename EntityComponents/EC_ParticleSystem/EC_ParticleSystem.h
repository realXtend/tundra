// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_ParticleSystem_EC_ParticleSystem_h
#define incl_EC_ParticleSystem_EC_ParticleSystem_h

#include "IComponent.h"
#include "IAttribute.h"
#include "ResourceInterface.h"
#include "Declare_EC.h"

namespace Ogre
{
    class ParticleSystem;
    class SceneNode;
}

namespace OgreRenderer
{
    class Renderer;
}

/**
<table class="header">
<tr>
<td>
<h2>ParticleSystem</h2>

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>QString: particleId
<div>Particle reosource asset id.</div> 
<li>bool: castShadows
<div>Deas particles cast shadows (mostly useless).</div> 
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

    bool HandleResourceEvent(event_id_t event_id, IEventData* data);

    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

    //! Particle reosource asset id.
    Q_PROPERTY(QString particleId READ getparticleId WRITE setparticleId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, particleId);

    //! Deas particles cast shadows (mostly useless).
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);

    //! Particles rendering distance.
    Q_PROPERTY(float renderingDistance READ getrenderingDistance WRITE setrenderingDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, renderingDistance);

    /*Attribute<QString> particleId_;
    Attribute<bool> castShadows_;
    Attribute<float> renderingDistance_;*/

public slots:
    //! Create a new particle system. System name will be same as component name.
    void CreateParticleSystem(const QString &systemName);

    //! Delete particle system.
    void DeleteParticleSystem();

private slots:
    void UpdateSignals();
    void AttributeUpdated(IAttribute *attribute);

private:
    explicit EC_ParticleSystem(IModule *module);
    ComponentPtr FindPlaceable() const;
    request_tag_t RequestResource(const std::string& id, const std::string& type);

    boost::weak_ptr<OgreRenderer::Renderer> renderer_;
    Ogre::ParticleSystem* particleSystem_;
    Ogre::SceneNode* node_;
    request_tag_t particle_tag_;
    event_category_id_t resource_event_category_;
};

#endif