/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Terrain.cpp
 *  @brief  Manages terrain-related logic.
 */

#include "StableHeaders.h"

#include "Terrain.h"
#include "TerrainDecoder.h"
#include "EnvironmentModule.h"

#include "EC_OgrePlaceable.h"
#include "Renderer.h"
#include "OgreTextureResource.h"
#include "OgreMaterialUtils.h"
#include "OgreConversionUtils.h"
#include "BitStream.h"
#include "SceneManager.h"
#include "NetworkEvents.h"
#include "ServiceManager.h"
#include "RexTypes.h"
#include "NetworkMessages/NetInMessage.h"
#include "Entity.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreIteratorWrappers.h>
#include <OgreTechnique.h>
#include <OgreMesh.h>
#include <OgreEntity.h>

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
            Environment::EnvironmentModule::LogWarning(ss.str());
            return;
        }

        std::stringstream ss;
        ss << "Texture \"" << texName << "\": width: " << tex->getWidth() << ", height: " << tex->getHeight() << ", mips: " << tex->getNumMipmaps();
        Environment::EnvironmentModule::LogDebug(ss.str());
    }

    //const char terrainMaterialName[] = "TerrainMaterial";
    const char terrainMaterialName[] = "Rex/TerrainPCF";
    //const char terrainMaterialName[] = "Rex/TerrainBool";
}

namespace Environment
{
    Terrain::Terrain(EnvironmentModule *owner)
    :owner_(owner)
    {
    }

    Terrain::~Terrain()
    {
    }

    /// Sets the texture of the material used to render terrain.
    void Terrain::SetTerrainMaterialTexture(int index, const char *textureName)
    {
        /// \todo Create a material that uses several terrain textures - for now only the first one is used.
        if (index < 0 || index > 4)
            return;

        Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
        Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(textureName).get());

        DebugDumpOgreTextureInfo(textureName);

        Ogre::MaterialPtr terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial(terrainMaterialName);
        assert(terrainMaterial.get());
        if(terrainMaterial.get())
        {
            OgreRenderer::SetTextureUnitOnMaterial(terrainMaterial, textureName, index);
            emit TerrainTextureChanged();
        }
        else
            EnvironmentModule::LogWarning("Ogre material " + std::string(terrainMaterialName) + " not found!");
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
                // see OpenSimToOgreCoordinateAxes.
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

    /// Creates Ogre geometry data for the single given patch, or updates the geometry for an existing
    /// patch if the associated Ogre resources already exist.
    void Terrain::GenerateTerrainGeometryForOnePatch(Scene::Entity &entity, EC_Terrain &terrain, EC_Terrain::Patch &patch)
    {
        OgreRenderer::RendererPtr renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
            return;

        Ogre::SceneNode *node = patch.node;
        bool firstTimeFill = (node == 0);
        if (!node)
        {
            CreateOgreTerrainPatchNode(node, patch.x, patch.y);
            patch.node = node;
        }
        assert(node);

        Ogre::MaterialPtr terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial(terrainMaterialName);

        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
        Ogre::ManualObject *manual = sceneMgr->createManualObject(renderer->GetUniqueObjectName());
        manual->setCastShadows(false);

        manual->clear();
        manual->estimateVertexCount(17*17);
        manual->estimateIndexCount(17*17*3*2);
        manual->begin(terrainMaterial->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

        const float vertexSpacingX = 1.f;
        const float vertexSpacingY = 1.f;
        const float patchSpacingX = 16 * vertexSpacingX;
        const float patchSpacingY = 16 * vertexSpacingY;
//        const Ogre::Vector3 patchOrigin(patch.y * patchSpacingY, 0.f, patch.x * patchSpacingX);
        const Ogre::Vector3 patchOrigin(patch.x * patchSpacingX, patch.y * patchSpacingY, 0.f);

        int curIndex = 0;

        const int stride = (patch.x + 1 >= terrain.cNumPatchesPerEdge) ? 16 : 17;

        const int patchSize = EC_Terrain::Patch::cNumVerticesPerPatchEdge;

        const float uScale = 1e-2f*13;
        const float vScale = 1e-2f*13;

        for(int y = 0; y <= patchSize; ++y)
            for(int x = 0; x <= patchSize; ++x)
            {
                if ((patch.x + 1 >= terrain.cNumPatchesPerEdge && x == patchSize) ||
                    (patch.y + 1 >= terrain.cNumPatchesPerEdge && y == patchSize))
                    continue;
                // These coordinates are directly generated to our Ogre coordinate system, i.e. are cycled from OpenSim XYZ -> our YZX.
                // see OpenSimToOgreCoordinateAxes.
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

        std::string mesh_name = renderer->GetUniqueObjectName();
        Ogre::MeshPtr terrainMesh = manual->convertToMesh(mesh_name);

        // Odd: destroyManualObject seems to leave behind a memory leak if we don't call manualObject->clear first.
        manual->clear();
        sceneMgr->destroyManualObject(manual);

        Ogre::Entity *ogre_entity = sceneMgr->createEntity(renderer->GetUniqueObjectName(), mesh_name);
        ogre_entity->setUserAny(Ogre::Any(&entity));
        ogre_entity->setCastShadows(false);

        // Explicitly destroy all attached MovableObjects previously bound to this terrain node.
        Ogre::SceneNode::ObjectIterator iter = node->getAttachedObjectIterator();
        while(iter.hasMoreElements())
        {
            Ogre::MovableObject *obj = iter.getNext();
            sceneMgr->destroyMovableObject(obj);
        }
        node->detachAllObjects();
        // Now attach the new built terrain mesh.
        node->attachObject(ogre_entity);

        patch.patch_geometry_dirty = false;

        emit HeightmapGeometryUpdated();
    }

    void Terrain::CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY)
    {
        OgreRenderer::RendererPtr renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
        {
            Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

            node = sceneMgr->createSceneNode();
            sceneMgr->getRootSceneNode()->addChild(node);
            
            const float vertexSpacingX = 1.f;
            const float vertexSpacingY = 1.f;
            const float patchSpacingX = 16 * vertexSpacingX;
            const float patchSpacingY = 16 * vertexSpacingY;
            const Ogre::Vector3 patchOrigin(patchX * patchSpacingX, patchY * patchSpacingY, 0.f);

            node->setPosition(patchOrigin);
        }
    }

    void Terrain::CreateOrUpdateTerrainPatchHeightData(const DecodedTerrainPatch &patch, int patchSize)
    {
        if (patch.heightData.size() < patchSize * patchSize)
        {
            EnvironmentModule::LogWarning("Not enough height map data to fill patch points!");
            return;
        }

        Scene::EntityPtr terrain = GetTerrainEntity().lock();
        EC_Terrain *terrainComponent = terrain->GetComponent<EC_Terrain>().get();
        assert(terrainComponent);
        EC_Terrain::Patch &scenePatch = terrainComponent->GetPatch(patch.header.x, patch.header.y);
        scenePatch.x = patch.header.x;
        scenePatch.y = patch.header.y;

        // Do a check to see if the height data actually changed. It seems that OpenSim server doesn't track this
        // and just stupidly sends all the patches after doing minor or no changes (or even if just changing the
        // terrain texture without changing the actual height data).
        bool heightDataChanged = false;
        if (scenePatch.heightData.size() != patch.heightData.size()) // If this patch did not exist at all?
            heightDataChanged = true;
        else
            for(size_t i = 0; i < scenePatch.heightData.size() && heightDataChanged == false; ++i)
                if (fabs(scenePatch.heightData[i] - patch.heightData[i]) > 1e-3f)
                    heightDataChanged = true;

        scenePatch.heightData = patch.heightData;
        // Flag the relevant GPU-side resources now to be dirty. We can't immediately regenerate them here since we need
        // slope and connectivity information from the neighboring patches as well, so we have to wait for later.
        // We need to mark the nearest 3x3 grid of patches dirty.
        if (heightDataChanged)
            for(int y = -1; y <= 1; ++y)
                for(int x = -1; x <= 1; ++x)
                {
                    int X = x + scenePatch.x;
                    int Y = y + scenePatch.y;
                    if (X >= 0 && X < EC_Terrain::cNumPatchesPerEdge &&
                        Y >= 0 && Y < EC_Terrain::cNumPatchesPerEdge)
                        terrainComponent->GetPatch(X, Y).patch_geometry_dirty = true;
                }

/*
        if (!scenePatch.node)
            CreateOgreTerrainPatchNode(scenePatch.node, scenePatch.x, scenePatch.y);

//        DebugGenerateTerrainVisData(scenePatch.node, patch, patchSize);

        if (terrainComponent->AllPatchesLoaded())
        {
            RequestTerrainTextures();
            GenerateTerrainGeometry(*terrainComponent);
        }
        */
    }

    void Terrain::RegenerateDirtyTerrainPatches()
    {
        PROFILE(RegenerateOgreTerrainGeom);
        Scene::EntityPtr terrain = GetTerrainEntity().lock();
        EC_Terrain *terrainComponent = terrain->GetComponent<EC_Terrain>().get();
        assert(terrainComponent);

        for(int y = 0; y < EC_Terrain::cNumPatchesPerEdge; ++y)
            for(int x = 0; x < EC_Terrain::cNumPatchesPerEdge; ++x)
            {
                EC_Terrain::Patch &scenePatch = terrainComponent->GetPatch(x, y);
                if (!scenePatch.patch_geometry_dirty || scenePatch.heightData.size() == 0)
                    continue;

                bool neighborsLoaded = true;

                const int neighbors[8][2] = 
                { 
                    { -1, -1 }, { -1, 0 }, { -1, 1 },
                    {  0, -1 },            {  0, 1 },
                    {  1, -1 }, {  1, 0 }, {  1, 1 }
                };

                for(int i = 0; i < 8; ++i)
                {
                    int nX = x + neighbors[i][0];
                    int nY = y + neighbors[i][1];
                    if (nX >= 0 && nX < EC_Terrain::cNumPatchesPerEdge &&
                        nY >= 0 && nY < EC_Terrain::cNumPatchesPerEdge &&
                        terrainComponent->GetPatch(nX, nY).heightData.size() == 0)
                    {
                        neighborsLoaded = false;
                        break;
                    }
                }

                if (neighborsLoaded)
                    GenerateTerrainGeometryForOnePatch(*terrain, *terrainComponent, scenePatch);
            }
    }

    void Terrain::RequestTerrainTextures()
    {
        OgreRenderer::RendererPtr renderer = owner_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
            for(int i = 0; i < num_terrain_textures; ++i)
                terrain_texture_requests_[i] = renderer->RequestResource(terrain_textures_[i], OgreRenderer::OgreTextureResource::GetTypeStatic());
    }

    void Terrain::OnTextureReadyEvent(Resource::Events::ResourceReady *tex)
    {
        assert(tex);
        for(int i = 0; i < num_terrain_textures; ++i)
        {
            if (tex->tag_ == terrain_texture_requests_[i])
            {
                int index = 0;
                if(terrain_textures_[i] == tex->id_.c_str())
                    index = i;

                SetTerrainMaterialTexture(index, tex->id_.c_str());
            }
        }
    }

    const RexTypes::RexAssetID &Terrain::GetTerrainTextureID(int index) const
    {
        if (index < 0)
            index = 0;
        if (index > num_terrain_textures)
            index = num_terrain_textures;
        return terrain_textures_[index];
    }

    /// Code adapted from libopenmetaverse.org project, TerrainCompressor.cs / TerrainManager.cs
    bool Terrain::HandleOSNE_LayerData(ProtocolUtilities::NetworkEventInboundData* data)
    {
        PROFILE(HandleOSNE_LayerData);

        ProtocolUtilities::NetInMessage &msg = *data->message;
        u8 layerID = msg.ReadU8();
        size_t sizeBytes = 0;
        const uint8_t *packedData = msg.ReadBuffer(&sizeBytes);
        if (!packedData)
            return false;
        ProtocolUtilities::BitStream bits(packedData, sizeBytes);
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
                CreateOrUpdateTerrainPatchHeightData(patches[i], header.patchSize);

            // Now that we have updated all the height map data for each patch, see if
            // we have enough of the patches loaded in to regenerate the GPU-side resources as well.
            RegenerateDirtyTerrainPatches();
            break;
        }
        case TPLayerWater:
            EnvironmentModule::LogDebug("Unhandled LayerData: Water");
            break;
        case TPLayerWind:
            EnvironmentModule::LogDebug("Unhandled LayerData: Wind");
            break;
        case TPLayerCloud:
            EnvironmentModule::LogDebug("Unhandled LayerData: Cloud");
            break;
        default:
            EnvironmentModule::LogDebug("Uknown LayerData: " + ToString(header.layerType));
            break;
        }
        return false;
    }

    void Terrain::SetTerrainTextures(const RexAssetID textures[num_terrain_textures])
    {
        bool texturesChanged = false;
        for(int i = 0; i < num_terrain_textures; ++i)
            if (terrain_textures_[i] != textures[i])
            {
                terrain_textures_[i] = textures[i];
                texturesChanged = true;
            }

        if (texturesChanged)
            RequestTerrainTextures();
    }

    void Terrain::SetTerrainHeightValues(const Real start_heights[num_terrain_textures], const Real height_ranges[num_terrain_textures])
    {
        for(int i = 0; i < num_terrain_textures; ++i)
        {
            start_heights_[i] = start_heights[i];
            height_ranges_[i] = height_ranges[i];
        }

        Ogre::MaterialPtr terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial(terrainMaterialName);
        if(!terrainMaterial.get())
        {
            EnvironmentModule::LogWarning("Cannot find " + std::string(terrainMaterialName) + "material.");
            return;
        }
        Ogre::Material::TechniqueIterator iter = terrainMaterial->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            assert(tech);
            if(tech->getName() != "TerrainPCF") // Skip code below if technique isn't right.
                continue;

            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                if(pass)
                {
                    Ogre::GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
                    if(!params.isNull())
                    {
                        Real lowest_height = 65535;
                        for(uint i = 0; i < num_terrain_textures; i++)
                        {
                            Real startHeight = start_heights[i];
                            Real endHeight = height_ranges[i];
                            if(startHeight < lowest_height)
                                lowest_height = startHeight;

                            Real heightDelta = endHeight - startHeight;
                            Ogre::Vector4 detailRegion(startHeight, startHeight+heightDelta/4, startHeight+((heightDelta*3)/4), endHeight);
                            params->setNamedConstant("detailRegion" + Ogre::StringConverter::toString(i), detailRegion);
                        }
                        lowest_height += 0.001f; //For some reason we need to increase lowest_height a little bit to get shader work properly.
                        params->setNamedConstant("lowestHeight", lowest_height);
                    }
                }
            }
        }

        emit TerrainTextureChanged();
    }

    const Real &Terrain::GetTerrainTextureStartHeight(int index) const
    {
        if(index > num_terrain_textures - 1) index = num_terrain_textures - 1;
        return start_heights_[index];
    }

    const Real &Terrain::GetTerrainTextureHeightRange(int index) const
    {
        if(index > num_terrain_textures - 1) index = num_terrain_textures - 1;
        return height_ranges_[index];
    }

    Real Terrain::GetLowestTerrainHeight()
    {
        Real small = 65535;
        Real current_value = 0;
        Scene::EntityPtr entity = GetTerrainEntity().lock();
        assert(entity.get());
        EC_Terrain *terrainComponent = entity->GetComponent<EC_Terrain>().get();
        if (!terrainComponent)
        {
            EnvironmentModule::LogWarning("EC_Terrain entity component is missing.");
            return 0;
        }
        int paches_per_edge = terrainComponent->cNumPatchesPerEdge;
        int patch_size = terrainComponent->cPatchSize;
        if(terrainComponent->AllPatchesLoaded())
        {
            for(uint i = 0; i < paches_per_edge; i++)
            {
                for(uint j = 0; j < paches_per_edge; j++)
                {
                    EC_Terrain::Patch patch = terrainComponent->GetPatch(i, j);
                    for(uint k = 0; k < patch_size; k++)
                    {
                        for(uint l = 0; l < patch_size; l++)
                        {
                            int index = (l % patch_size) * patch_size + (k % patch_size);
                            current_value = patch.heightData[index];
                            if(current_value < small)
                                small = current_value;
                        }
                    }
                }
            }
        }
        return small;
    }

    void Terrain::FindCurrentlyActiveTerrain()
    {
        Scene::ScenePtr scene = owner_->GetFramework()->GetDefaultWorldScene();
        for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
        {
            Scene::Entity &entity = **iter;
            EC_Terrain *terrainComponent = entity.GetComponent<EC_Terrain>().get();
            if (terrainComponent)
                cachedTerrainEntity_ = scene->GetEntity(entity.GetId());
        }
    }

    Scene::EntityWeakPtr Terrain::GetTerrainEntity() const
    {
        return cachedTerrainEntity_;
    }
}
