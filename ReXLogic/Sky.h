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

/// Sky component
class Sky
{
public:
    Sky(RexLogicModule *owner);
    virtual ~Sky();

    /// The OGRE SkyBox has 6 textures: front, back, left, right, top and bottom.
    static const int skyBoxTextureCount = 6;

    /// Handler for the "RexSky" generic message.
    /// @param data Event data pointer.
    bool HandleRexGM_RexSky(OpenSimProtocol::NetworkEventInboundData* data);

    /** Update sky info.
     *
     * @param type Type of the sky: box, dome, plane or none. 
     * @param images List of image uuid's for the sky.
     * @param curvature If sky type is dome, curvature of the dome.
     * @param tiling If sky type is dome, tiling of the dome.
     */
    void UpdateSky(OgreRenderer::SkyType type, const std::vector<std::string> &images, float curvature, float tiling);
    
    /// Creates the default sky.
    /// @param show Visibility of the sky.
    void CreateDefaultSky(bool show = true);
    
    /// Request the texture assets used for the sky.
    void RequestSkyTextures();

    /// Called whenever a texture is loaded so it can be attached to the sky.
    /// @param Resource pointer to the texture.
    void OnTextureReadyEvent(Resource::Events::ResourceReady *tex);
        
    /// Set the sky texture for Skydome or Skyplane.
    /// @param texture_id
    void SetSkyTexture(const RexUUID texture_id);

    /// Sets the SkyBox textures.
    /// @param textures array of texture UUID's.
    void SetSkyBoxTextures(const RexUUID textures[skyBoxTextureCount]);

    /// Looks through all the entities in RexLogic's currently active scene to find the Sky
    /// entity. Caches it internally. Use GetSkyEntity to obtain it afterwards.    
    void FindCurrentlyActiveSky();
    
    /// @return The scene entity that represents the sky in the currently active world.
    Scene::EntityWeakPtr GetSkyEntity();
    
private:
    Sky(const Sky &);
    void operator =(const Sky &);
    
    RexLogicModule *owner_;
    
    /// The type of the sky (none, box, dome or plane).
    OgreRenderer::SkyType type_;
    
    /// Whether sky is enabled by this component.
    bool skyEnabled_;
    
    /// Texture resource request tags for skybox.
    Core::request_tag_t skyBoxTextureRequests_[skyBoxTextureCount];
    
    /// Texture resource request tag for skydome.
    Core::request_tag_t skyDomeTextureRequest_;
    
    /// Texture resource request tag for skyplane.
    Core::request_tag_t skyPlaneTextureRequest_;
        
    /// UUID's of the texture assets the skybox uses for rendering. Should be stored per-scene.
    RexUUID skyBoxTextures_[skyBoxTextureCount];

    /// List of skybox image names (UUID's) as strings.
    std::vector<std::string> skyBoxImages_;
    
    /// UUID of the texture asset the skydome uses for rendering. Should be stored per-scene.
    RexUUID skyDomeTexture_;

    /// UUID of the texture asset the skyplane uses for rendering. Should be stored per-scene.
    RexUUID skyPlaneTexture_;
    
    /// Keeps count of the skybox images.
    size_t skyBoxImageCount_;
    
    /// Keeps count of the currently to-be-added skybox images.
    size_t currentSkyBoxImageCount_;
           
    /// The scene entity that represents the sky in the currently active world.
    Scene::EntityWeakPtr cachedSkyEntity_;
};

} // namespace RexLogic

#endif
