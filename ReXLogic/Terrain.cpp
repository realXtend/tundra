/// @file Terrain.cpp
/// @brief Manages Terrain-related Rex logic.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

// Ogre renderer -specific.
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>

#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/Renderer.h"

#include "BitStream.h"
#include "TerrainDecoder.h"
#include "RexLogicModule.h"
#include "Terrain.h"

namespace RexLogic
{
    Terrain::Terrain(RexLogicModule *owner_)
    :owner(owner_)
    {
    }

    Terrain::~Terrain()
    {
    }

    void Terrain::DebugCreateTerrainVisData(const DecodedTerrainPatch &patch, int patchSize)
    {
        if (patch.heightData.size() < patchSize * patchSize)
        {
            RexLogicModule::LogWarning("Not enough height map data to fill patch points!");
            return;
        }

        OgreRenderer::Renderer *renderer = owner->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

        ///\todo Quick W.I.P Ogre object naming, refactor. -jj.
        static int c = 0;
        std::stringstream ss;
        ss.clear();
        ss << "terrain " << c++;
        Ogre::ManualObject *manual = sceneMgr->createManualObject(ss.str());
        manual->begin("AmbientWhite", Ogre::RenderOperation::OT_LINE_LIST);

        const float vertexSpacingX = 1.f;
        const float vertexSpacingY = 1.f;
        const float patchSpacingX = 16 * vertexSpacingX;
        const float patchSpacingY = 16 * vertexSpacingY;
        const Ogre::Vector3 patchOrigin(patch.header.x * patchSpacingX, 0.f, patch.header.y * patchSpacingY);
        const float heightScale = 1.f;
        for(int y = 0; y+1 < patchSize; ++y)
            for(int x = 0; x+1 < patchSize; ++x)
            {
                Ogre::Vector3 a = patchOrigin + Ogre::Vector3(vertexSpacingX * x,     heightScale * patch.heightData[y*patchSize+x],   vertexSpacingY * y);
                Ogre::Vector3 b = patchOrigin + Ogre::Vector3(vertexSpacingX * (x+1), heightScale * patch.heightData[y*patchSize+x+1], vertexSpacingY * y);
                Ogre::Vector3 c = patchOrigin + Ogre::Vector3(vertexSpacingX * x,     heightScale * patch.heightData[(y+1)*patchSize+x], vertexSpacingY * (y+1));
                Ogre::Vector3 d = patchOrigin + Ogre::Vector3(vertexSpacingX * (x+1), heightScale * patch.heightData[(y+1)*patchSize+x+1], vertexSpacingY * (y+1));

                manual->position(a);
                manual->position(b);
                manual->position(a);
                manual->position(c);
//                manual->position(a);
//                manual->position(d);

                if (x+2 == patchSize)
                {
                    manual->position(b);
                    manual->position(d);
                }
                if (y+2 == patchSize)
                {
                    manual->position(c);
                    manual->position(d);
                }
            }

        manual->end();
        manual->setDebugDisplayEnabled(true);
       
        Ogre::SceneNode *node = sceneMgr->createSceneNode();
        sceneMgr->getRootSceneNode()->addChild(node);
        node->attachObject(manual);
    }

    /// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
    bool Terrain::HandleOSNE_LayerData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage &msg = *data->message;
        u8 layerID = msg.ReadU8();
        size_t sizeBytes = 0;
        const uint8_t *packedData = msg.ReadBuffer(&sizeBytes);
        BitStream bits(packedData, sizeBytes);
        TerrainPatchGroupHeader header;

        header.stride = bits.ReadBits(16);
        header.patchSize = bits.ReadBits(8);
        header.layerType = bits.ReadBits(8);

        switch(header.layerType)
        {
        case TPLayerLand:
        {
            std::vector<DecodedTerrainPatch> patches;
            DecompressLand(patches, bits, header);
            for(size_t i = 0; i < patches.size(); ++i)
                DebugCreateTerrainVisData(patches[i], header.patchSize);
            break;
        }
        default:
            ///\todo Log out warning - unhandled packet type.
            break;
        }
        return false;
    }

}