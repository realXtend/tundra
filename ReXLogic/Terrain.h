/// @file Terrain.h
/// @brief Packet structure definitions for terrain LayerData packets.
///        Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef RexLogic_Terrain_h
#define RexLogic_Terrain_h 

namespace RexLogic
{

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
struct TerrainPatchGroupHeader
{
    Core::u16 stride;
    Core::u8 patchSize;
    Core::u8 layerType;
};

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
struct TerrainPatchHeader
{
    Core::u8 quantWBits;
    float dcOffset;
    Core::u16 range;
    Core::u8 x;
    Core::u8 y;
    Core::uint wordBits; // Not present in the stream.
};

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
enum TerrainPatchLayerType
{
    TPLayerLand = 0x4C,
    TPLayerWater = 0x57,
    TPLayerWind = 0x37,
    TPLayerCloud = 0x38
};

void DecompressLand(BitStream &bits, const TerrainPatchGroupHeader &groupHeader);

}

#endif
