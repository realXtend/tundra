/// @file TerrainDecoder.h
/// @brief Packet structure definitions for terrain LayerData packets.
///        Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef Environment_Terrain_h
#define Environment_Terrain_h 

#include "BitStream.h"

namespace Environment
{

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
struct TerrainPatchGroupHeader
{
    u16 stride;
    u8 patchSize;
    u8 layerType;
};

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
struct TerrainPatchHeader
{
    u8 quantWBits;
    float dcOffset;
    u16 range;
    u8 x;
    u8 y;
    uint wordBits; // Not present in the stream.
};

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
enum TerrainPatchLayerType
{
    TPLayerLand = 0x4C,
    TPLayerWater = 0x57,
    TPLayerWind = 0x37,
    TPLayerCloud = 0x38
};

/// Data structure to contain the output data from the terrain IDCT decoder.
struct DecodedTerrainPatch
{
    std::vector<float> heightData;
    TerrainPatchHeader header;
};

/// Decompresses a single patch of terrain height data from a LayerData packet.
/// @param patches [out] The resulting patch data will be output here.
/// @param bits [in] The LayerData packet, of which the Patch Group Header has already been read.
/// @param groupHeader 
void DecompressLand(std::vector<DecodedTerrainPatch> &patches, ProtocolUtilities::BitStream &bits, const TerrainPatchGroupHeader &groupHeader);

}

#endif
