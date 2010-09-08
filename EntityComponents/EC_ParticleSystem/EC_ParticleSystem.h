// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_ParticleSystem_EC_ParticleSystem_h
#define incl_EC_ParticleSystem_EC_ParticleSystem_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
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
    class EC_OgrePlaceable;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}

namespace Foundation
{
    class Framework;
}

namespace Foundation
{
    class ModuleInterface;
    typedef boost::shared_ptr<Foundation::ComponentInterface> ComponentPtr;
}

class EC_ParticleSystem : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_ParticleSystem);
    Q_OBJECT
public:
    ~EC_ParticleSystem();

    virtual bool IsSerializable() const { return true; }

    bool HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);

    //! Create a new particle system. System name will be same as component name.
    /*! \return true if successful
    */
    void CreateParticleSystem(const QString &systemName);
    void DeleteParticleSystem();
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    Foundation::Attribute<QString>      particleId_;
    Foundation::Attribute<bool>         castShadows_;
    Foundation::Attribute<float>         renderingDistance_;

public slots:
    void AttributeUpdated(Foundation::ComponentInterface *component, Foundation::AttributeInterface *attribute);

private slots:
    void UpdateSignals();

private:
    explicit EC_ParticleSystem(Foundation::ModuleInterface *module);
    Foundation::ComponentPtr FindPlaceable() const;
    request_tag_t RequestResource(const std::string& id, const std::string& type);

    OgreRenderer::RendererWeakPtr renderer_;
    Ogre::ParticleSystem* particleSystem_;
    Ogre::SceneNode* node_;
    request_tag_t particle_tag_;
    event_category_id_t resource_event_category_;
};

#endif