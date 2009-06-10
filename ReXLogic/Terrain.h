/// @file Terrain.h
/// @brief Manages Terrain-related Rex logic.
/// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Terrain_h
#define incl_Terrain_h

#include "EC_Terrain.h"

namespace Resource
{
namespace Events
{
class ResourceReady;
}
}

namespace RexLogic
{

//! Handles the logic related to the OpenSim Terrain. Note - partially lacks support for multiple scenes - the Terrain object is not instantiated
//! per-scene, but it contains data that should be stored per-scene. This doesn't affect anything unless we will some day actually have several scenes.
class Terrain
{
public:
    Terrain(RexLogicModule *owner_);
    ~Terrain();

    //! Called to handle an OpenSim LayerData packet.
    //! Decodes terrain data from a LayerData packet and generates terrain patches accordingly.
    bool HandleOSNE_LayerData(OpenSimProtocol::NetworkEventInboundData* data);

    //! The OpenSim terrain has a hardcoded size of four textures. When/if we lift that, change the amount here or remove altogether if dynamic.
    static const int num_terrain_textures = 4;

    void SetTerrainTextures(const RexAssetID textures[num_terrain_textures]);

    void RequestTerrainTextures();

    //! Looks through all the entities in RexLogic's currently active scene to find the Terrain
    //! entity. Caches it internally. Use GetTerrainEntity to obtain it afterwards.
    void FindCurrentlyActiveTerrain();

    //! @return The scene entity that represents the terrain in the currently active world.
    Scene::EntityWeakPtr GetTerrainEntity();

    //! Called whenever a texture is loaded so it can be attached to the terrain.
    void OnTextureReadyEvent(Resource::Events::ResourceReady *tex);
private:
    RexLogicModule *owner_;

    Core::request_tag_t terrain_texture_requests_[num_terrain_textures];

    //! UUID's of the texture assets the terrain uses for rendering. Should be stored per-scene.
    RexAssetID terrain_textures_[num_terrain_textures];

    Scene::EntityWeakPtr cachedTerrainEntity_;

    void CreateOrUpdateTerrainPatch(const DecodedTerrainPatch &patch, int patchSize);

    void CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY);

    void GenerateTerrainGeometryForOnePatch(EC_Terrain &terrain, EC_Terrain::Patch &patch);
    void GenerateTerrainGeometry(EC_Terrain &terrain);
    void DebugGenerateTerrainVisData(Ogre::SceneNode *node, const DecodedTerrainPatch &patch, int patchSize);

    void SetTerrainMaterialTexture(int index, const char *textureName);
};

}

#endif
