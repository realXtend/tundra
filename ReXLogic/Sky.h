// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Sky_h
#define incl_RexLogicModule_Sky_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "EC_OgreSky.h"

namespace Resource
{
namespace Events
{
class ResourceReady;
}
}

namespace RexLogic
{

//! Sky component
class Sky
{
public:
    Sky(RexLogicModule *owner);
    virtual ~Sky();

    //! The OGRE skybox has 6 textures: front, back, left, right, top and bottom.
    static const int skyboxTextureCount = 6;

    //! Create the default sky.
    void CreateDefaultSky(bool show);

    bool HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);        
    
    void RequestSkyboxTextures();

    void OnTextureReadyEvent(Resource::Events::ResourceReady *tex);
    
    /// Handler for the "RexSky" generic message.
    bool HandleRexGM_RexSky(OpenSimProtocol::NetworkEventInboundData* data);
    
    //! Looks through all the entities in RexLogic's currently active scene to find the Sky
    //! entity. Caches it internally. Use GetSkyEntity to obtain it afterwards.    
    void FindCurrentlyActiveSky();
    
    //! @return The scene entity that represents the sky in the currently active world.
    Scene::EntityWeakPtr GetSkyEntity();    
    
private:
    RexLogicModule *owner_;
    
    //! whether sky enabled by this component
    bool skyEnabled_;
    
    //! Texture resource request tags.
    Core::request_tag_t skyboxTextureRequests_[skyboxTextureCount];
    
    //! UUID's of the texture assets the terrain uses for rendering. Should be stored per-scene.
    RexUUID skyboxTextures_[skyboxTextureCount];
    
    //! The scene entity that represents the sky in the currently active world.
    Scene::EntityWeakPtr cachedSkyEntity_;
    
};

} // namespace RexLogic

#endif
