/// @file Terrain.cpp
/// @brief Manages Terrain-related Rex logic.
/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

// Ogre renderer -specific.
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreIteratorWrappers.h>
#include <OgreTechnique.h>

#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/Renderer.h"
#include "../OgreRenderingModule/OgreTextureResource.h"
#include "../OgreRenderingModule/OgreMaterialUtils.h"
#include "../OgreRenderingModule/OgreConversionUtils.h"

#include "BitStream.h"
#include "TerrainDecoder.h"
#include "RexLogicModule.h"
#include "Terrain.h"
#include "SceneManager.h"

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

namespace
{
    void DebugDumpOgreTextureInfo(const char *texName)
    {
        Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
        Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(texName).get());
        if (!tex)
        {
            std::stringstream ss;
            ss << "Ogre Texture \"" << texName << "\" not found!";
            RexLogicModule::LogWarning(ss.str());
            return;
        }

        std::stringstream ss;
        ss << "Texture \"" << texName << "\": width: " << tex->getWidth() << ", height: " << tex->getHeight() << ", mips: " << tex->getNumMipmaps();
        RexLogicModule::LogDebug(ss.str());
    }

    const char terrainMaterialName[] = "TerrainMaterial";
    //const char terrainMaterialName[] = "Rex/TerrainPCF";
    //const char terrainMaterialName[] = "Rex/TerrainBool";
}

    /// Sets the texture of the material used to render terrain.
    void Terrain::SetTerrainMaterialTexture(int index, const char *textureName)
    {
        /// \todo Create a material that uses several terrain textures - for now only the first one is used.
        if (index != 0)
            return;

        Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
        Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(textureName).get());

        DebugDumpOgreTextureInfo(textureName);

        Ogre::MaterialPtr terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial(terrainMaterialName);
        //Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
        //Ogre::MaterialPtr terrainMaterial = mm.getByName(terrainMaterialName);
        
        assert(terrainMaterial.get());
        OgreRenderer::SetTextureUnitOnMaterial(terrainMaterial, textureName);

//        while(iter->
//        terrainMaterial->
//        Ogre::MaterialPtr newMaterial = material->clone(materialName);
//        newMaterial->setAmbient(r, g, b);
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
        const Ogre::Vector3 patchOrigin(patch.header.x * patchSpacingX, patch.header.y * patchSpacingY, 0.f);
//        const Ogre::Vector3 patchOrigin(patch.header.y * patchSpacingY, 0.f, patch.header.x * patchSpacingX);
        const float heightScale = 1.f;
        for(int y = 0; y+1 < patchSize; ++y)
            for(int x = 0; x+1 < patchSize; ++x)
            {
                // These coordinates are directly generated to our Ogre coordinate system, i.e. are cycled from OpenSim XYZ -> our YZX.
                // see Core::OpenSimToOgreCoordinateAxes.
//                Ogre::Vector3 a = patchOrigin + Ogre::Vector3(vertexSpacingY * y,     heightScale * patch.heightData[y*patchSize+x], vertexSpacingX * x);
//                Ogre::Vector3 b = patchOrigin + Ogre::Vector3(vertexSpacingY * y, heightScale * patch.heightData[y*patchSize+x+1], vertexSpacingX * (x+1));
//                Ogre::Vector3 c = patchOrigin + Ogre::Vector3(vertexSpacingY * (y+1),     heightScale * patch.heightData[(y+1)*patchSize+x], vertexSpacingX * x);
//                Ogre::Vector3 d = patchOrigin + Ogre::Vector3(vertexSpacingY * (y+1), heightScale * patch.heightData[(y+1)*patchSize+x+1], vertexSpacingX * (x+1));

                Ogre::Vector3 a = Ogre::Vector3(vertexSpacingX * x, vertexSpacingY * y,     heightScale * patch.heightData[y*patchSize+x]);
                Ogre::Vector3 b = Ogre::Vector3(vertexSpacingX * (x+1), vertexSpacingY * y, heightScale * patch.heightData[y*patchSize+x+1]);
                Ogre::Vector3 c = Ogre::Vector3(vertexSpacingX * x, vertexSpacingY * (y+1),     heightScale * patch.heightData[(y+1)*patchSize+x]);
                Ogre::Vector3 d = Ogre::Vector3(vertexSpacingX * (x+1), vertexSpacingY * (y+1), heightScale * patch.heightData[(y+1)*patchSize+x+1]);

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

    void Terrain::GenerateTerrainGeometryForOnePatch(EC_Terrain &terrain, EC_Terrain::Patch &patch)
    {
        Ogre::SceneNode *node = patch.node;
        assert(node);
        assert(node->numAttachedObjects() == 1);
        Ogre::MaterialPtr terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial(terrainMaterialName);

        Ogre::ManualObject *manual = dynamic_cast<Ogre::ManualObject*>(node->getAttachedObject(0));
        manual->clear(); /// \note For optimization, could use beginUpdate.
        manual->begin(terrainMaterial->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

        const float vertexSpacingX = 1.f;
        const float vertexSpacingY = 1.f;
        const float patchSpacingX = 16 * vertexSpacingX;
        const float patchSpacingY = 16 * vertexSpacingY;
//        const Ogre::Vector3 patchOrigin(patch.y * patchSpacingY, 0.f, patch.x * patchSpacingX);
        const Ogre::Vector3 patchOrigin(patch.x * patchSpacingX, patch.y * patchSpacingY, 0.f);

        int curIndex = 0;

        int stride = (patch.x + 1 >= terrain.cNumPatchesPerEdge) ? 16 : 17;

        const int patchSize = 16;

        const float uScale = 1e-2f;
        const float vScale = 1e-2f;

        for(int y = 0; y <= patchSize; ++y)
            for(int x = 0; x <= patchSize; ++x)
            {
                if ((patch.x + 1 >= terrain.cNumPatchesPerEdge && x == patchSize) ||
                    (patch.y + 1 >= terrain.cNumPatchesPerEdge && y == patchSize))
                    continue;
                // These coordinates are directly generated to our Ogre coordinate system, i.e. are cycled from OpenSim XYZ -> our YZX.
                // see Core::OpenSimToOgreCoordinateAxes.
                Ogre::Vector3 pos;
// Ogre:                pos.x = vertexSpacingY * y;
// Ogre:                pos.z = vertexSpacingX * x;
                pos.x = vertexSpacingX * x;
                pos.y = vertexSpacingY * y;

                EC_Terrain::Patch *thisPatch;
                int X = x;
                int Y = y;
                if (x < patchSize && y < patchSize)
                {
                    thisPatch = &patch;

                    if ((patch.x + 1 < terrain.cNumPatchesPerEdge || x+1 < patchSize) &&
                        (patch.y + 1 < terrain.cNumPatchesPerEdge || y+1 < patchSize))
                    {
                        manual->index(curIndex);
                        manual->index(curIndex+1);
                        manual->index(curIndex+stride);

                        manual->index(curIndex+1);
                        manual->index(curIndex+stride+1);
                        manual->index(curIndex+stride);
                    }
                }
                else if (x == patchSize && y == patchSize)
                {
                    thisPatch = &terrain.GetPatch(patch.x + 1, patch.y + 1);
                    X = 0;
                    Y = 0;
                }
                else if (x == patchSize)
                {
                    thisPatch = &terrain.GetPatch(patch.x + 1, patch.y);
                    X = 0;
                }
                else // (y == patchSize)
                {
                    thisPatch = &terrain.GetPatch(patch.x, patch.y + 1);
                    Y = 0;
                }

// Ogre:        pos.y = thisPatch->heightData[Y*patchSize+X];
                pos.z = thisPatch->heightData[Y*patchSize+X];

                manual->position(pos);
                manual->normal(OgreRenderer::ToOgreVector3(terrain.CalculateNormal(thisPatch->x, thisPatch->y, X, Y)));
// Ogre:                manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.z + pos.z) * vScale);
                manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.y + pos.y) * vScale);
                ++curIndex;
            }

        manual->end();
//        manual->setDebugDisplayEnabled(true);
    }

    void Terrain::GenerateTerrainGeometry(EC_Terrain &terrain) 
    {
        for(int y = 0; y < terrain.cNumPatchesPerEdge; ++y)
            for(int x = 0; x < terrain.cNumPatchesPerEdge; ++x)
            {
                EC_Terrain::Patch &patch = terrain.GetPatch(x, y);

                if (!patch.node)
                    CreateOgreTerrainPatchNode(patch.node, x, y);

                GenerateTerrainGeometryForOnePatch(terrain, patch);
            }
    }

    void Terrain::CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
        {
            Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

            Ogre::ManualObject *manual = sceneMgr->createManualObject(renderer->GetUniqueObjectName());
            manual->setCastShadows(false);

            node = sceneMgr->createSceneNode();
            sceneMgr->getRootSceneNode()->addChild(node);
            node->attachObject(manual);
            
            const float vertexSpacingX = 1.f;
            const float vertexSpacingY = 1.f;
            const float patchSpacingX = 16 * vertexSpacingX;
            const float patchSpacingY = 16 * vertexSpacingY;
            const Ogre::Vector3 patchOrigin(patchX * patchSpacingX, patchY * patchSpacingY, 0.f);

            node->setPosition(patchOrigin);
        }
    }

    void Terrain::CreateOrUpdateTerrainPatch(const DecodedTerrainPatch &patch, int patchSize)
    {
        if (patch.heightData.size() < patchSize * patchSize)
        {
            RexLogicModule::LogWarning("Not enough height map data to fill patch points!");
            return;
        }

        Scene::EntityPtr terrain = GetTerrainEntity().lock();
        EC_Terrain *terrainComponent = checked_static_cast<EC_Terrain*>(terrain->GetComponent("EC_Terrain").get());
        assert(terrainComponent);
        EC_Terrain::Patch &scenePatch = terrainComponent->GetPatch(patch.header.x, patch.header.y);
        scenePatch.x = patch.header.x;
        scenePatch.y = patch.header.y;
        scenePatch.heightData = patch.heightData;

        if (!scenePatch.node)
            CreateOgreTerrainPatchNode(scenePatch.node, scenePatch.x, scenePatch.y);

//        DebugGenerateTerrainVisData(scenePatch.node, patch, patchSize);

        if (terrainComponent->AllPatchesLoaded())
        {
            RequestTerrainTextures();
            GenerateTerrainGeometry(*terrainComponent);
        }
    }

    void Terrain::RequestTerrainTextures()
    {
        boost::weak_ptr<OgreRenderer::Renderer> w_renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        boost::shared_ptr<OgreRenderer::Renderer> renderer = w_renderer.lock();

        for(int i = 0; i < num_terrain_textures; ++i)
            terrain_texture_requests_[i] = renderer->RequestResource(terrain_textures_[i], OgreRenderer::OgreTextureResource::GetTypeStatic());
    }

    void Terrain::OnTextureReadyEvent(Resource::Events::ResourceReady *tex)
    {
        assert(tex);

        for(int i = 0; i < num_terrain_textures; ++i)
            if (tex->tag_ == terrain_texture_requests_[i])
                SetTerrainMaterialTexture(i, tex->id_.c_str());
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

    void Terrain::SetTerrainTextures(const RexAssetID textures[num_terrain_textures])
    {
        for(int i = 0; i < num_terrain_textures; ++i)
            terrain_textures_[i] = textures[i];
    }

    void Terrain::FindCurrentlyActiveTerrain()
    {
        Scene::ScenePtr scene = owner_->GetCurrentActiveScene();
        for(Scene::SceneManager::iterator iter = scene->begin();
            iter != scene->end(); ++iter)
        {
            Scene::Entity &entity = **iter;
            Foundation::ComponentInterfacePtr terrainComponent = entity.GetComponent("EC_Terrain");
            if (terrainComponent.get())
            {
                cachedTerrainEntity_ = scene->GetEntity(entity.GetId());
            }
        }
    }

    Scene::EntityWeakPtr Terrain::GetTerrainEntity()
    {
        return cachedTerrainEntity_;
    }
}
