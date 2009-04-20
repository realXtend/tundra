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

    void DebugCreateTerrainVisData(const DecodedTerrainPatch &patch, int patchSize);

    //! Decodes terrain data from a LayerData packet and generates terrain patches accordingly.
    bool HandleOSNE_LayerData(OpenSimProtocol::NetworkEventInboundData* data);

private:
    RexLogicModule *owner;

};

}

#endif
