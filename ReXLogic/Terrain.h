/// @file Terrain.h
/// @brief Manages Terrain-related Rex logic.
/// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Terrain_h
#define incl_Terrain_h

namespace RexLogic
{

class Terrain
{
public:
    Terrain(RexLogicModule *owner_);
    ~Terrain();

    //! Called to handle an OpenSim LayerData packet.
    //! Decodes terrain data from a LayerData packet and generates terrain patches accordingly.
    bool HandleOSNE_LayerData(OpenSimProtocol::NetworkEventInboundData* data);

    //! Looks through all the entities in RexLogic's currently active scene to find the Terrain
    //! entity. Caches it internally. Use GetTerrainEntity to obtain it afterwards.
    void FindCurrentlyActiveTerrain();

    //! @return The scene entity that represents the terrain in the currently active world.
    Foundation::EntityWeakPtr GetTerrainEntity();
private:
    RexLogicModule *owner_;

    Foundation::EntityWeakPtr cachedTerrainEntity_;

    void CreateOrUpdateTerrainPatch(const DecodedTerrainPatch &patch, int patchSize);

    void CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY);

    void DebugGenerateTerrainVisData(Ogre::SceneNode *node, const DecodedTerrainPatch &patch, int patchSize);

};

}

#endif
