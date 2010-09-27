/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Terrain.h
 *  @brief  Manages terrain-related logic.
 */

#ifndef incl_EnvironmentModule_Terrain_h
#define incl_EnvironmentModule_Terrain_h

#include "EC_Terrain.h"
#include "EnvironmentModuleApi.h"
#include "RexTypes.h"

#include <QObject>

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
}

namespace ProtocolUtilities
{
    class NetworkEventInboundData;
}

namespace Environment
{
    class EC_Terrain;
    class EnvironmentModule;
    struct DecodedTerrainPatch;

    //! Handles the logic related to the OpenSim Terrain. Note - partially lacks support for multiple scenes - the Terrain object is not instantiated
    //! per-scene, but it contains data that should be stored per-scene. This doesn't affect anything unless we will some day actually have several scenes.
    //! \ingroup EnvironmentModuleClient.
    class ENVIRONMENT_MODULE_API Terrain: public QObject
    {
        Q_OBJECT

    public:
        //! Constructor.
        //! @param owner_ owner of this object.
        Terrain(EnvironmentModule *owner_);

        //! Destructor
        ~Terrain();

        //! Called to handle an OpenSim LayerData packet.
        //! Decodes terrain data from a LayerData packet and generates terrain patches accordingly.
        bool HandleOSNE_LayerData(ProtocolUtilities::NetworkEventInboundData* data);

        //! The OpenSim terrain has a hardcoded size of four textures. When/if we lift that, change the amount here or remove altogether if dynamic.
        static const int num_terrain_textures = 4;

        //! Sets the new terrain texture UUIDs that are used for this terrain. Places
        //! new resource requests to the asset handler if any of the textures have changed.
        void SetTerrainTextures(const RexTypes::RexAssetID textures[num_terrain_textures]);

        /*! 
         * Set new terrain height ranges, that will be used with terrain texture shader.
         * @param start_heights array contains 4 different terrain texture start height value in meters.
         * @param height_ranges array contains 4 different terrain texture end height value in meters.
         */
        void SetTerrainHeightValues(const float start_heights[num_terrain_textures], const float height_ranges[num_terrain_textures]);

        //! Request new terrain textures from the server.
        void RequestTerrainTextures();

        //! Looks through all the entities in RexLogic's currently active scene to find the Terrain
        //! entity. Caches it internally. Use GetTerrainEntity to obtain it afterwards.
        void FindCurrentlyActiveTerrain();

        //! @return The scene entity that represents the terrain in the currently active world.
        Scene::EntityWeakPtr GetTerrainEntity() const;

        //! @return The terrain component of the currently active world.
        EC_Terrain *GetTerrainComponent();

        //! Called whenever a texture is loaded so it can be attached to the terrain.
        void OnTextureReadyEvent(Resource::Events::ResourceReady *tex);

        //! Get terrain texture ids
        //! @param index index of texture id range[0-3].
        const RexTypes::RexAssetID &GetTerrainTextureID(int index) const;

        //! Get terrain texture start height in meters.
        //! @param switch texture height value need to return range[0-3].
        const float &GetTerrainTextureStartHeight(int index) const;

        //! Get terrain texture height ranges in meters.
        //! @param switch texture height value need to return range[0-3].
        const float &GetTerrainTextureHeightRange(int index) const;

    signals:
        //! Signal is sended when height map values have changed.
        void HeightmapGeometryUpdated();

        //! Signal is sended when some of the terrain textures have changed.
        void TerrainTextureChanged();

    private:
        void CreateOrUpdateTerrainPatchHeightData(const DecodedTerrainPatch &patch, int patchSize);
//        void RegenerateDirtyTerrainPatches();
//        void CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY);
//        void GenerateTerrainGeometryForOnePatch(Scene::Entity &entity, EC_Terrain &terrain, EC_Terrain::Patch &patch);
        void GenerateTerrainGeometry(EC_Terrain &terrain);
        void GenerateTerrainGeometryForSinglePatch(EC_Terrain &terrain, int patchX, int patchY);
        void DebugGenerateTerrainVisData(Ogre::SceneNode *node, const DecodedTerrainPatch &patch, int patchSize);
//        void SetTerrainMaterialTexture(int index, const char *textureName);

        /// Sets the terrain parameters to OpenSim-specific hardcoded values.
        void SetupOpenSimTerrainParameters();

        /// Environment module's pointer.
        EnvironmentModule *owner_;

        /// Request tags for new terrain textures.
        request_tag_t terrain_texture_requests_[num_terrain_textures];

        /// UUID's of the texture assets the terrain uses for rendering. Should be stored per-scene.
        RexTypes::RexAssetID terrain_textures_[num_terrain_textures];

        /// Array of terrain texture start heights.
        float start_heights_[num_terrain_textures];

        /// Array of terrain texture end heights.
        float height_ranges_[num_terrain_textures];

        Scene::EntityWeakPtr cachedTerrainEntity_;
    };
}

#endif
