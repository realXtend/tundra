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

#include "EC_Terrain.h"

using namespace Core;

namespace RexLogic
{
    Terrain::Terrain(RexLogicModule *owner)
    :owner_(owner)
    {
    }

    Terrain::~Terrain()
    {
    }

    void Terrain::DebugGenerateTerrainVisData(Ogre::SceneNode *node, const DecodedTerrainPatch &patch, int patchSize)
    {
        assert(node->numAttachedObjects() == 1);
        Ogre::ManualObject *manual = dynamic_cast<Ogre::ManualObject*>(node->getAttachedObject(0));
        manual->clear(); /// \note For optimization, could use beginUpdate.
        manual->begin("AmbientWhite", Ogre::RenderOperation::OT_LINE_LIST);

        const float vertexSpacingX = 1.f;
        const float vertexSpacingY = 1.f;
        const float patchSpacingX = 16 * vertexSpacingX;
        const float patchSpacingY = 16 * vertexSpacingY;
        const Ogre::Vector3 patchOrigin(patch.header.y * patchSpacingY, 0.f, patch.header.x * patchSpacingX);
        const float heightScale = 1.f;
        for(int y = 0; y+1 < patchSize; ++y)
            for(int x = 0; x+1 < patchSize; ++x)
            {
                // These coordinates are directly generated to our Ogre coordinate system, i.e. are cycled from OpenSim XYZ -> our YZX.
                // see Core::OpenSimToOgreCoordinateAxes.
                Ogre::Vector3 a = patchOrigin + Ogre::Vector3(vertexSpacingY * y,     heightScale * patch.heightData[y*patchSize+x], vertexSpacingX * x);
                Ogre::Vector3 b = patchOrigin + Ogre::Vector3(vertexSpacingY * y, heightScale * patch.heightData[y*patchSize+x+1], vertexSpacingX * (x+1));
                Ogre::Vector3 c = patchOrigin + Ogre::Vector3(vertexSpacingY * (y+1),     heightScale * patch.heightData[(y+1)*patchSize+x], vertexSpacingX * x);
                Ogre::Vector3 d = patchOrigin + Ogre::Vector3(vertexSpacingY * (y+1), heightScale * patch.heightData[(y+1)*patchSize+x+1], vertexSpacingX * (x+1));

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
    }

    void Terrain::CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY)
    {
        OgreRenderer::Renderer *renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

        std::stringstream ss;
        ss.clear();
        ss << "TerrainPatch " << patchX << ", " << patchY;
        Ogre::ManualObject *manual = sceneMgr->createManualObject(ss.str());

        node = sceneMgr->createSceneNode();
        sceneMgr->getRootSceneNode()->addChild(node);
        node->attachObject(manual);
    }

    void Terrain::CreateOrUpdateTerrainPatch(const DecodedTerrainPatch &patch, int patchSize)
    {
        if (patch.heightData.size() < patchSize * patchSize)
        {
            RexLogicModule::LogWarning("Not enough height map data to fill patch points!");
            return;
        }

        Foundation::EntityPtr terrain = GetTerrainEntity().lock();
        EC_Terrain *terrainComponent = checked_static_cast<EC_Terrain*>(terrain->GetComponent("EC_Terrain").get());
        assert(terrainComponent);
        EC_Terrain::Patch &scenePatch = terrainComponent->GetPatch(patch.header.x, patch.header.y);
        scenePatch.x = patch.header.x;
        scenePatch.y = patch.header.y;
        scenePatch.heightData = patch.heightData;

        if (!scenePatch.node)
            CreateOgreTerrainPatchNode(scenePatch.node, scenePatch.x, scenePatch.y);

        DebugGenerateTerrainVisData(scenePatch.node, patch, patchSize);
    }

    /// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
    bool Terrain::HandleOSNE_LayerData(OpenSimProtocol::NetworkEventInboundData* data)
    {
        NetInMessage &msg = *data->message;
        u8 layerID = msg.ReadU8();
        size_t sizeBytes = 0;
        const uint8_t *packedData = msg.ReadBuffer(&sizeBytes);
        OpenSimProtocol::BitStream bits(packedData, sizeBytes);
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
                CreateOrUpdateTerrainPatch(patches[i], header.patchSize);
            break;
        }
        default:
            ///\todo Log out warning - unhandled packet type.
            break;
        }
        return false;
    }

    void Terrain::FindCurrentlyActiveTerrain()
    {
        Foundation::ScenePtr scene = owner_->GetCurrentActiveScene();
        for(Foundation::SceneInterface::EntityIterator iter = scene->begin();
            iter != scene->end(); ++iter)
        {
            Foundation::EntityInterface &entity = *iter;
            Foundation::ComponentInterfacePtr terrainComponent = entity.GetComponent("EC_Terrain");
            if (terrainComponent.get())
            {
                cachedTerrainEntity_ = scene->GetEntity(entity.GetId());
            }
        }
    }

    Foundation::EntityWeakPtr Terrain::GetTerrainEntity()
    {
        return cachedTerrainEntity_;
    }
}
