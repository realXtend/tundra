/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TerrainDecoder.cpp
 *  @brief  Performs IDCT decompression on terrain height map data.
 *          Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
 */

#include "StableHeaders.h"

#include "Poco/Net/DatagramSocket.h" // To get htons etc.

#include "BitStream.h"
#include "TerrainDecoder.h"
#include "EnvironmentModule.h"

namespace Environment
{

namespace
{
const int cEndOfPatches = 97; ///< Magic number that denotes in a LayerData header that there are no more patches present in the packet.
const float OO_SQRT2 = 0.7071067811865475244008443621049f;

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
/// Stores precomputed tables of coefficients needed in the IDCT transform.
class IDCTPrecomputationTables
{
public:
    IDCTPrecomputationTables()
    {
        BuildDequantizeTable16();
        BuildQuantizeTable16();
        SetupCosines16();
        BuildCopyMatrix16();
    }

    float dequantizeTable16[16*16];
    float cosineTable16[16*16];
    int copyMatrix16[16*16];
    float quantizeTable16[16*16];

    void BuildDequantizeTable16()
    {
        for (int j = 0; j < 16; j++)
            for (int i = 0; i < 16; i++)
                dequantizeTable16[j*16 + i] = 1.0f + 2.0f * (float)(i + j);
    }

    void BuildQuantizeTable16()
    {
        for (int j = 0; j < 16; j++)
            for (int i = 0; i < 16; i++)
                quantizeTable16[j*16 + i] = 1.0f / (1.0f + 2.0f * ((float)i + (float)j));
    }

    void SetupCosines16()
    {
        const float hposz = PI * 0.5f / 16.0f;

        for (int u = 0; u < 16; u++)
            for (int n = 0; n < 16; n++)
                cosineTable16[u*16 + n] = (float)cosf((2.0f * (float)n + 1.0f) * (float)u * hposz);
    }

    void BuildCopyMatrix16()
    {
        bool diag = false;
        bool right = true;
        int i = 0;
        int j = 0;
        int count = 0;

        while (i < 16 && j < 16)
        {
            copyMatrix16[j * 16 + i] = count++;

            if (!diag)
            {
                if (right)
                {
                    if (i < 16 - 1) i++;
                    else j++;

                    right = false;
                    diag = true;
                }
                else
                {
                    if (j < 16 - 1) j++;
                    else i++;

                    right = true;
                    diag = true;
                }
            }
            else
            {
                if (right)
                {
                    i++;
                    j--;
                    if (i == 16 - 1 || j == 0) diag = false;
                }
                else
                {
                    i--;
                    j++;
                    if (j == 16 - 1 || i == 0) diag = false;
                }
            }
        }
    }
};

/// These tables will be used by the IDCT routines. The ctor builds the tables and only this instance is accessed by the routines.
const IDCTPrecomputationTables precompTables;

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
/// Extracts and returns the header for a single terrain patch in the LayerData stream.
TerrainPatchHeader DecodePatchHeader(ProtocolUtilities::BitStream &bits)
{
   TerrainPatchHeader header;
   header.quantWBits = bits.ReadBits(8);
   if (header.quantWBits == cEndOfPatches)
       return header;

   u32 val = bits.ReadBits(32);
   header.dcOffset = *reinterpret_cast<float*>(&val); // Apparently the height coordinate of the lowest patch.
   header.range = bits.ReadBits(16); // The difference between lowest and highest point on the patch.
   u32 patchIDs = bits.ReadBits(10);
   header.x = patchIDs >> 5;
   header.y = patchIDs & 31;
   header.wordBits = (uint)((header.quantWBits & 0x0f) + 2); // This is a bit odd - apparently this field is not present in the header?

   return header;
}

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
/// @param patches [out] The resulting patch data will be output here. The size of this buffer must be >= size*size.
/// @param size The number of points in the patch in one direction (patches are square). 
void DecodeTerrainPatch(int *patches, ProtocolUtilities::BitStream &bits, const TerrainPatchHeader &header, int size)
{
    for(int i = 0; i < size * size; ++i)
    {
        if (bits.BitsLeft() == 0)
        {
            std::stringstream ss;
            ss << "Out of bits when decoding terrain vertex " << i << "!";
            EnvironmentModule::LogInfo(ss.str());
            for(; i < size * size; ++i)
                patches[i] = 0;
            return;
        }
        bool v = bits.ReadBit(); // 'Patches present' flag?
        if (!v)
        {
            patches[i] = 0;
            continue;
        }

        v = bits.ReadBit(); // 'End of patch data' flag?
        if (!v)
        {
            for(; i < size * size; ++i)
                patches[i] = 0;
            return;
        }

        bool signNegative = bits.ReadBit();
        u32 data = (u32)bits.ReadBits(header.wordBits);
        patches[i] = signNegative ? -(s32)data : (s32)data;
    }
}

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
/// Performs IDCT on a single column of 16 elements of data. (stride assumed to be 16 elements)
void IDCTColumn16(const float *linein, float *lineout, int column)
{
    float total;
    const int cStride = 16;

    for (int n = 0; n < 16; n++)
    {
        total = OO_SQRT2 * linein[column];

        for (int u = 1; u < 16; u++)
            total += linein[u*cStride + column] * precompTables.cosineTable16[u*cStride + n];

        lineout[16 * n + column] = total;
    }
}

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
/// Performs IDCT on a single row of 16 elements of data.
void IDCTLine16(const float *linein, float *lineout, int line)
{
    const float oosob = 2.0f / 16.0f;
    int lineSize = line * 16;
    float total;

    for (int n = 0; n < 16; n++)
    {
        total = OO_SQRT2 * linein[lineSize];

        for (int u = 1; u < 16; u++)
        {
            total += linein[lineSize + u] * precompTables.cosineTable16[u * 16 + n];
        }

        lineout[lineSize + n] = total * oosob;
    }
}

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
void DecompressTerrainPatch(std::vector<float> &output, int *patchData, const TerrainPatchHeader &patchHeader, const TerrainPatchGroupHeader &groupHeader)
{
    std::vector<float> block(groupHeader.patchSize * groupHeader.patchSize);
    output.clear();
    output.resize(groupHeader.patchSize * groupHeader.patchSize);

    int prequant = (patchHeader.quantWBits >> 4) + 2;
    int quantize = 1 << prequant;
    float ooq = 1.0f / (float)quantize;
    float mult = ooq * (float)patchHeader.range;
    float addval = mult * (float)(1 << (prequant - 1)) + patchHeader.dcOffset;

    if (groupHeader.patchSize != 16)
    {
        EnvironmentModule::LogWarning("TerrainDecoder:DecompressTerrainPatch: Unsupported patch size present!");
        return;
    }

    for(int n = 0; n < 16 * 16; n++)
        block[n] = patchData[precompTables.copyMatrix16[n]] * precompTables.dequantizeTable16[n];

    std::vector<float> ftemp(16*16);

    for (int o = 0; o < 16; o++)
        IDCTColumn16(&block[0], &ftemp[0], o);
    for (int o = 0; o < 16; o++)
        IDCTLine16(&ftemp[0], &block[0], o);

    for (int j = 0; j < block.size(); j++)
        output[j] = block[j] * mult + addval;
}

} // ~unnamed namespace

/// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
void DecompressLand(std::vector<DecodedTerrainPatch> &patches, ProtocolUtilities::BitStream &bits, const TerrainPatchGroupHeader &groupHeader)
{
    while(bits.BitsLeft() > 0)
    {
        DecodedTerrainPatch patch;
        patch.header = DecodePatchHeader(bits);

        if (patch.header.quantWBits == cEndOfPatches)
            break;

        const int cPatchesPerEdge = 16;

        // The MSB of header.x and header.y are unused, or used for some other purpose?
        if (patch.header.x >= cPatchesPerEdge || patch.header.y >= cPatchesPerEdge)
        {
            EnvironmentModule::LogWarning("TerrainDecoder:DecompressLand: Invalid patch data!");
            return;
        }

        int patchData[16*16];
        DecodeTerrainPatch(patchData, bits, patch.header, groupHeader.patchSize);

        DecompressTerrainPatch(patch.heightData, patchData, patch.header, groupHeader);

        patches.push_back(patch); ///\todo Can optimize out a copy of heavy struct by push_backing early in the loop.
    }
}

}
