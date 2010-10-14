// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Terrain.h"

#include "Renderer.h"
#include "IModule.h"
#include "ServiceManager.h"

#include <Ogre.h>
#include "OgreMaterialUtils.h"
#include "OgreConversionUtils.h"

#include <utility>

using namespace std;
using namespace OgreRenderer;

namespace OgreRenderer
{
typedef boost::shared_ptr<Renderer> RendererPtr;
}

namespace Environment
{

EC_Terrain::EC_Terrain(IModule* module) :
    IComponent(module->GetFramework()),
    nodeTransformation(this, "Transform"),
    xPatches(this, "Grid Width"),
    yPatches(this, "Grid Height"),
    material(this, "Material"),
    texture0(this, "Texture 0"),
    texture1(this, "Texture 1"),
    texture2(this, "Texture 2"),
    texture3(this, "Texture 3"),
    texture4(this, "Texture 4"),
    heightMap(this, "Heightmap"),
    uScale(this, "Tex. U scale"),
    vScale(this, "Tex. V scale"),
    patchWidth(1),
    patchHeight(1),
    rootNode(0)
{
    QObject::connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));

    xPatches.Set(1, AttributeChange::LocalOnly);
    yPatches.Set(1, AttributeChange::LocalOnly);
    patches.resize(1);
    MakePatchFlat(0, 0, 0.f);
    uScale.Set(0.13f, AttributeChange::LocalOnly);
    vScale.Set(0.13f, AttributeChange::LocalOnly);
}

EC_Terrain::~EC_Terrain()
{
    Destroy();
}

void EC_Terrain::UpdateSignals()
{
    disconnect(this, SLOT(AttributeUpdated(IAttribute *)));

    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            this, SLOT(AttributeUpdated(IAttribute*)));

    connect(this, SIGNAL(TerrainSizeChanged(int)), this, SLOT(OnTerrainSizeChanged()));
    connect(this, SIGNAL(MaterialChanged(QString)), this, SLOT(OnMaterialChanged()));
    connect(this, SIGNAL(TextureChanged(QString)), this, SLOT(OnTextureChanged()));
}

void EC_Terrain::OnTerrainSizeChanged()
{
    ResizeTerrain(xPatches.Get(), yPatches.Get());
}

void EC_Terrain::OnMaterialChanged()
{
    for(int y = 0; y < patchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
            UpdateTerrainPatchMaterial(x, y);
}

void EC_Terrain::OnTextureChanged()
{
    PROFILE(EC_Terrain_OnTextureChanged);
    SetTerrainMaterialTexture(0, texture0.Get().toStdString().c_str());
    SetTerrainMaterialTexture(1, texture1.Get().toStdString().c_str());
    SetTerrainMaterialTexture(2, texture2.Get().toStdString().c_str());
    SetTerrainMaterialTexture(3, texture3.Get().toStdString().c_str());
    SetTerrainMaterialTexture(4, texture4.Get().toStdString().c_str());
}

void EC_Terrain::MakePatchFlat(int x, int y, float heightValue)
{
    Patch &patch = GetPatch(x, y);
    patch.heightData.clear();
    patch.heightData.insert(patch.heightData.end(), cPatchSize*cPatchSize, heightValue);
    patch.patch_geometry_dirty = true;
}

void EC_Terrain::ResizeTerrain(int newPatchWidth, int newPatchHeight)
{
    PROFILE(EC_Terrain_ResizeTerrain);

    const int maxPatchSize = 32;
    // Do an artificial limit to a preset N patches per side. (This limit is way too large already for the current terrain vertex LOD management.)
    newPatchWidth = max(1, min(maxPatchSize, newPatchWidth));
    newPatchHeight = max(1, min(maxPatchSize, newPatchHeight));

    if (newPatchWidth == patchWidth && newPatchHeight == patchHeight)
        return;

    // If the width changes, we need to also regenerate the old right-most column to generate the new seams. (If we are shrinking, this is not necessary)
    if (patchWidth < newPatchWidth)
        for(int y = 0; y < patchHeight; ++y)
            GetPatch(patchWidth-1, y).patch_geometry_dirty = true;

    // If the height changes, we need to also regenerate the old top-most row to generate the new seams. (If we are shrinking, this is not necessary)
    if (patchHeight < newPatchHeight)
        for(int x = 0; x < patchWidth; ++x)
            GetPatch(x, patchHeight-1).patch_geometry_dirty = true;

    // First delete all the patches that will not be part of the newly-sized terrain (user shrinked the terrain in one or two dimensions)
    for(int y = newPatchHeight; y < patchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
            DestroyPatch(x, y);
    for(int x = newPatchWidth; x < patchWidth; ++x) // We hav esome overlap here with above, but it's ok since DestroyPatch is benign.
        for(int y = 0; y < patchHeight; ++y)
            DestroyPatch(x, y);

    // Now create the new terrain patch storage and copy the old height values over.
    std::vector<Patch> newPatches(newPatchWidth * newPatchHeight);
    for(int y = 0; y < min(patchHeight, newPatchHeight); ++y)
        for(int x = 0; x < min(patchWidth, newPatchWidth); ++x)
            newPatches[y * newPatchWidth + x] = GetPatch(x, y);
    patches = newPatches;
    int oldPatchWidth = patchWidth;
    int oldPatchHeight = patchHeight;
    patchWidth = newPatchWidth;
    patchHeight = newPatchHeight;

    // Init any new patches to flat planes with the given fixed height.

    const float initialPatchHeight = 0.f;

    for(int y = oldPatchHeight; y < newPatchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
            MakePatchFlat(x, y, initialPatchHeight);
    for(int x = oldPatchWidth; x < newPatchWidth; ++x) // We have some overlap here with above, but it's ok since DestroyPatch is benign.
        for(int y = 0; y < patchHeight; ++y)
            MakePatchFlat(x, y, initialPatchHeight);

    // Tell each patch which coordinate in the grid they lie in.
    for(int y = 0; y < patchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
        {
            GetPatch(x,y).x = x;
            GetPatch(x,y).y = y;
        }

    // Re-do all the geometry on the GPU.
    RegenerateDirtyTerrainPatches();
}

//! Emitted when some of the attributes has been changed.
void EC_Terrain::AttributeUpdated(IAttribute *attribute)
{
    std::string changedAttribute = attribute->GetNameString();

    if (changedAttribute == xPatches.GetNameString() || changedAttribute == yPatches.GetNameString())
    {
        ResizeTerrain(xPatches.Get(), yPatches.Get());
    }
    else if (changedAttribute == nodeTransformation.GetNameString())
    {
        if (!rootNode)
            CreateRootNode();
        if (!rootNode)
            return;

        const Transform &tm = nodeTransformation.Get();

        Ogre::Matrix3 rot_new;
        rot_new.FromEulerAnglesXYZ(Ogre::Degree(tm.rotation.x), Ogre::Degree(tm.rotation.y), Ogre::Degree(tm.rotation.z));
        Ogre::Quaternion q_new(rot_new);
        rootNode->setOrientation(Ogre::Quaternion(rot_new));

        rootNode->setPosition(tm.position.x, tm.position.y, tm.position.z);

        rootNode->setScale(tm.scale.x, tm.scale.y, tm.scale.z);

        ///\todo support tm change.
    }
    else if (changedAttribute == material.GetNameString())
    {
        SetTerrainMaterialTexture(0, texture0.Get().toStdString().c_str());
        SetTerrainMaterialTexture(1, texture1.Get().toStdString().c_str());
        SetTerrainMaterialTexture(2, texture2.Get().toStdString().c_str());
        SetTerrainMaterialTexture(3, texture3.Get().toStdString().c_str());
        SetTerrainMaterialTexture(4, texture4.Get().toStdString().c_str());

        for(int y = 0; y < patchHeight; ++y)
            for(int x = 0; x < patchWidth; ++x)
                UpdateTerrainPatchMaterial(x, y);

        ///\todo Delete the old unused material.
    }
    else if (changedAttribute == texture0.GetNameString()) SetTerrainMaterialTexture(0, texture0.Get().toStdString().c_str());
    else if (changedAttribute == texture1.GetNameString()) SetTerrainMaterialTexture(1, texture1.Get().toStdString().c_str());
    else if (changedAttribute == texture2.GetNameString()) SetTerrainMaterialTexture(2, texture2.Get().toStdString().c_str());
    else if (changedAttribute == texture3.GetNameString()) SetTerrainMaterialTexture(3, texture3.Get().toStdString().c_str());
    else if (changedAttribute == texture4.GetNameString()) SetTerrainMaterialTexture(4, texture4.Get().toStdString().c_str());
    else if (changedAttribute == heightMap.GetNameString())
    {
        if (currentHeightmapAssetSource.trimmed() != heightMap.Get().trimmed())
        LoadFromFile(heightMap.Get().toStdString().c_str());
    }
    else if (changedAttribute == uScale.GetNameString() || changedAttribute == vScale.GetNameString())
    {
        // Re-do all the geometry on the GPU.
        DirtyAllTerrainPatches();
        RegenerateDirtyTerrainPatches();
    }

    ///\todo Delete the old unused textures.
}

/// Releases all GPU resources used for the given patch.
void EC_Terrain::DestroyPatch(int x, int y)
{
    if (x >= patchWidth || y >= patchHeight || x < 0 || y < 0)
        return;

    assert(GetFramework());
    if (!GetFramework())
        return;

    boost::shared_ptr<OgreRenderer::Renderer> renderer = GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>().lock();
    if (!renderer) // Oops! Inconvenient dtor order - can't delete our own stuff since we can't get an instance to the owner.
        return;

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    if (!sceneMgr) // Oops! Same as above.
        return;

    EC_Terrain::Patch &patch = GetPatch(x, y);

    if (patch.node)
    {
        sceneMgr->getRootSceneNode()->removeChild(patch.node);
        patch.node->detachAllObjects();
        sceneMgr->destroySceneNode(patch.node);
        patch.node = 0;
    }
    if (patch.entity)
    {
        sceneMgr->destroyEntity(patch.entity);
        patch.entity = 0;
    }

    // If there exists a previously generated GPU Mesh resource, delete it before creating a new one.
    if (patch.meshGeometryName.length() > 0)
    {
        try
        {
            Ogre::MeshManager::getSingleton().remove(patch.meshGeometryName);
        }
        catch (...) {}
        patch.meshGeometryName = "";
    }
}

void EC_Terrain::Destroy()
{
    for(int y = 0; y < patchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
            DestroyPatch(x, y);

    if (!GetFramework())
        return;

    boost::shared_ptr<OgreRenderer::Renderer> renderer = GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>().lock();
    if (!renderer) // Oops! Inconvenient dtor order - can't delete our own stuff since we can't get an instance to the owner.
        return;

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    if (!sceneMgr) // Oops! Same as above.
        return;

    if (rootNode)
    {
        sceneMgr->destroySceneNode(rootNode);
        rootNode = 0;
    }

    ///\todo Clear up materials and textures.
}

float EC_Terrain::GetPoint(int x, int y) const
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x >= cPatchSize * patchWidth)
        x = cPatchSize * patchWidth - 1;
    if (y >= cPatchSize * patchHeight)
        y = cPatchSize * patchHeight - 1;

    return GetPatch(x / cPatchSize, y / cPatchSize).heightData[(y % cPatchSize) * cPatchSize + (x % cPatchSize)];
}

float EC_Terrain::GetInterpolatedHeightValue(float x, float y) const
{
    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    xFloor = clamp(xFloor, 0, PatchWidth() * cPatchSize);
    xCeil = clamp(xCeil, 0, PatchWidth() * cPatchSize);
    yFloor = clamp(yFloor, 0, PatchHeight() * cPatchSize);
    yCeil = clamp(yCeil, 0, PatchHeight() * cPatchSize);

    float xFrac = fmod(x, 1.f);
    float yFrac = fmod(y, 1.f);
    float h1;
    if (xFrac + yFrac >= 1.f)
    {
        //if xFrac >= yFrac
        h1 = GetPoint(xCeil, yCeil);
        xFrac = 1.f - xFrac;
        yFrac = 1.f - yFrac;
    }
    else
        h1 = GetPoint(xFloor, yFloor);

    float h2 = GetPoint(xCeil, yFloor);
    float h3 = GetPoint(xFloor, yCeil);
    return h1 * (1.f - xFrac - yFrac) + h2 * xFrac + h3 * yFrac;
}

Vector3df EC_Terrain::CalculateNormal(int x, int y, int xinside, int yinside)
{
    int px = x * cPatchSize + xinside;
    int py = y * cPatchSize + yinside;

    int xNext = clamp(px+1, 0, patchWidth * cPatchSize - 1);
    int yNext = clamp(py+1, 0, patchHeight * cPatchSize - 1);
    int xPrev = clamp(px-1, 0, patchWidth * cPatchSize - 1);
    int yPrev = clamp(py-1, 0, patchHeight * cPatchSize - 1);

    float x_slope = GetPoint(xPrev, py) - GetPoint(xNext, py);
    if ((px <= 0) || (px >= patchWidth * cPatchSize))
        x_slope *= 2;
    float y_slope = GetPoint(px, yPrev) - GetPoint(px, yNext);
    if ((py <= 0) || (py >= patchHeight * cPatchSize))
        y_slope *= 2;

    Vector3df normal(x_slope, y_slope, 2.0);
    normal.normalize();
    return normal;
}

void EC_Terrain::SaveToFile(QString filename)
{
    if (patchWidth * patchHeight != patches.size())
        return; ///\todo Log out error. The EC_Terrain is in inconsistent state. Cannot save.

    FILE *handle = fopen(filename.toStdString().c_str(), "wb");
    if (!handle)
    {
        ///\todo Log out error!
        return;
    }

    const u32 xPatches = patchWidth;
    const u32 yPatches = patchHeight;
    fwrite(&xPatches, sizeof(u32), 1, handle);
    fwrite(&yPatches, sizeof(u32), 1, handle);

    assert(sizeof(float) == 4);

    for(int i = 0; i < xPatches*yPatches; ++i)
    {
        if (patches[i].heightData.size() < cPatchSize*cPatchSize)
            patches[i].heightData.resize(cPatchSize*cPatchSize);

        fwrite(&patches[i].heightData[0], sizeof(float), cPatchSize*cPatchSize, handle); ///< \todo Check read error.
    }
    fclose(handle);

}

void EC_Terrain::LoadFromFile(QString filename)
{
    filename = filename.trimmed();

    FILE *handle = fopen(filename.toStdString().c_str(), "rb");
    if (!handle)
    {
        ///\todo Log out error!
        return;
    }
    u32 xPatches = 0;
    u32 yPatches = 0;
    fread(&xPatches, sizeof(u32), 1, handle); ///< \todo Check read error.
    fread(&yPatches, sizeof(u32), 1, handle); ///< \todo Check read error.

    // Load all the data from the file to an intermediate buffer first, so that we can first see
    // if the file is not broken, and reject it without losing the old terrain.
    std::vector<Patch> newPatches(xPatches*yPatches);

    // Initialize the new height data structure.
    for(int y = 0; y < yPatches; ++y)
        for(int x = 0; x < xPatches; ++x)
        {
            newPatches[y*xPatches+x].x = x;
            newPatches[y*xPatches+x].y = y;
        }

    assert(sizeof(float) == 4);

    // Load the new data.
    for(size_t i = 0; i < newPatches.size(); ++i)
    {
        newPatches[i].heightData.resize(cPatchSize*cPatchSize);
        newPatches[i].patch_geometry_dirty = true;
        fread(&newPatches[i].heightData[0], sizeof(float), cPatchSize*cPatchSize, handle); ///< \todo Check read error.
    }

    fclose(handle);

    // The terrain asset loaded ok. We are good to set that terrain as the active terrain.

    Destroy();

    currentHeightmapAssetSource = filename;
    patches = newPatches;
    patchWidth = xPatches;
    patchHeight = yPatches;

    // Re-do all the geometry on the GPU.
    RegenerateDirtyTerrainPatches();

    // Set the new number of patches this terrain has. These changes only need to be done locally, since the other
    // peers have loaded the terrain from the same file, and they will also locally do this change. This change is also
    // performed in "batched" mode, i.e. first the values are set, and only after that the signals are emitted manually.
    this->xPatches.Set(patchWidth, AttributeChange::Disconnected);
    this->yPatches.Set(patchHeight, AttributeChange::Disconnected);

    this->xPatches.Changed(AttributeChange::LocalOnly);
    this->yPatches.Changed(AttributeChange::LocalOnly);
}

void EC_Terrain::SetTerrainMaterialTexture(int index, const char *textureName)
{
    if (index < 0 || index > 4)
        return;

//    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
//    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(textureName).get());

    Ogre::MaterialPtr terrainMaterial = Ogre::MaterialManager::getSingleton().getByName(material.Get().toStdString().c_str());
    if (!terrainMaterial.get())
    {
//        EnvironmentModule::LogWarning("Ogre material " + material.Get().toStdString() + " not found!");
        return;
    }
//    Ogre::MaterialPtr terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial(terrainMaterialName);
//    assert(terrainMaterial.get());
 //   if(terrainMaterial.get())
//    {
        OgreRenderer::SetTextureUnitOnMaterial(terrainMaterial, textureName, index);
//        emit TerrainTextureChanged(); ///\todo Regression here. Re-enable this so that the EnvironmentEditor texture viewer can see the textures?
//    }
//    else
//        EnvironmentModule::LogWarning("Ogre material " + std::string(terrainMaterialName) + " not found!");
}

void EC_Terrain::UpdateTerrainPatchMaterial(int patchX, int patchY)
{
    Patch &patch = GetPatch(patchX, patchY);

    if (!patch.entity)
        return;

    for(int i = 0; i < patch.entity->getNumSubEntities(); ++i)
    {
        Ogre::SubEntity *sub = patch.entity->getSubEntity(i);
        if (sub)
            sub->setMaterialName(material.Get().toStdString().c_str());
    }
}

/// Creates Ogre geometry data for the single given patch, or updates the geometry for an existing
/// patch if the associated Ogre resources already exist.
void EC_Terrain::GenerateTerrainGeometryForOnePatch(int patchX, int patchY)
{
    PROFILE(EC_Terrain_GenerateTerrainGeometryForOnePatch);

    EC_Terrain::Patch &patch = GetPatch(patchX, patchY);

    boost::shared_ptr<Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
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

    Ogre::MaterialPtr terrainMaterial = Ogre::MaterialManager::getSingleton().getByName(material.Get().toStdString().c_str());
    if (!terrainMaterial.get()) // If we could not find the material we were supposed to use, just use the default system terrain material.
        terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial("Rex/TerrainPCF");

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    Ogre::ManualObject *manual = sceneMgr->createManualObject(renderer->GetUniqueObjectName());
    manual->setCastShadows(false);

    manual->clear();
    manual->estimateVertexCount((cPatchSize+1)*(cPatchSize+1));
    manual->estimateIndexCount((cPatchSize+1)*(cPatchSize+1)*3*2);
    manual->begin(terrainMaterial->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

    const float vertexSpacingX = 1.f;
    const float vertexSpacingY = 1.f;
    const float patchSpacingX = cPatchSize * vertexSpacingX;
    const float patchSpacingY = cPatchSize * vertexSpacingY;
//        const Ogre::Vector3 patchOrigin(patch.y * patchSpacingY, 0.f, patch.x * patchSpacingX);
    const Ogre::Vector3 patchOrigin(patch.x * patchSpacingX, patch.y * patchSpacingY, 0.f);

    int curIndex = 0;

    const int cPatchVertexWidth = cPatchSize; // The number of vertices in the patch in horizontal direction. We use the fixed value of cPatchSize==16.
    const int cPatchVertexHeight = cPatchSize; // The number of vertices in the patch in vertical  direction. We use the fixed value of cPatchSize==16.
    // If we assume each patch is 16x16 vertices, then all the internal patches will get a 17x17 grid, since we need to connect seams.
    // But, the outermost patch row and column at the terrain edge will not have this, since they do not need to connect to a next patch.
    // This is the vertex stride for the terrain.
    const int stride = (patch.x + 1 >= patchWidth) ? cPatchVertexWidth : (cPatchVertexWidth+1);

    const float uScale = this->uScale.Get();
    const float vScale = this->vScale.Get();

    for(int y = 0; y <= cPatchVertexHeight; ++y)
        for(int x = 0; x <= cPatchVertexWidth; ++x)
        {
            if ((patch.x + 1 >= patchWidth && x == cPatchVertexWidth) ||
                (patch.y + 1 >= patchHeight && y == cPatchVertexHeight))
                continue; // We are at the single corner-most vertex of the whole terrain. That is to be skipped.

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
            if (x < cPatchVertexWidth && y < cPatchVertexHeight)
            {
                thisPatch = &patch;

                if ((patch.x + 1 < patchWidth || x+1 < cPatchVertexWidth) &&
                    (patch.y + 1 < patchHeight || y+1 < cPatchVertexHeight))
                {
                    manual->index(curIndex);
                    manual->index(curIndex+1);
                    manual->index(curIndex+stride);

                    manual->index(curIndex+1);
                    manual->index(curIndex+stride+1);
                    manual->index(curIndex+stride);
                }
            }
            else if (x == cPatchVertexWidth && y == cPatchVertexHeight)
            {
                thisPatch = &GetPatch(patch.x + 1, patch.y + 1);
                X = 0;
                Y = 0;
            }
            else if (x == cPatchVertexWidth)
            {
                thisPatch = &GetPatch(patch.x + 1, patch.y);
                X = 0;
            }
            else // (y == patchHeight)
            {
                thisPatch = &GetPatch(patch.x, patch.y + 1);
                Y = 0;
            }

// Ogre:        pos.y = thisPatch->heightData[Y*patchSize+X];
            pos.z = thisPatch->heightData[Y*cPatchVertexWidth+X];

            manual->position(pos);
            manual->normal(OgreRenderer::ToOgreVector3(CalculateNormal(thisPatch->x, thisPatch->y, X, Y)));
// Ogre:                manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.z + pos.z) * vScale);
            manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.y + pos.y) * vScale);
            ++curIndex;
        }

    manual->end();

    // If there exists a previously generated GPU Mesh resource, delete it before creating a new one.
    if (patch.meshGeometryName.length() > 0)
    {
        try
        {
            Ogre::MeshManager::getSingleton().remove(patch.meshGeometryName);
        }
        catch (...) {}
    }

    patch.meshGeometryName = renderer->GetUniqueObjectName();
    Ogre::MeshPtr terrainMesh = manual->convertToMesh(patch.meshGeometryName);

    // Odd: destroyManualObject seems to leave behind a memory leak if we don't call manualObject->clear first.
    manual->clear();
    sceneMgr->destroyManualObject(manual);

    patch.entity = sceneMgr->createEntity(renderer->GetUniqueObjectName(), patch.meshGeometryName);
    patch.entity->setUserAny(Ogre::Any(parent_entity_));
    patch.entity->setCastShadows(false);
    // Set UserAny also on subentities
    for (uint i = 0; i < patch.entity->getNumSubEntities(); ++i)
        patch.entity->getSubEntity(i)->setUserAny(patch.entity->getUserAny());

    // Explicitly destroy all attached MovableObjects previously bound to this terrain node.
    Ogre::SceneNode::ObjectIterator iter = node->getAttachedObjectIterator();
    while(iter.hasMoreElements())
    {
        Ogre::MovableObject *obj = iter.getNext();
        sceneMgr->destroyMovableObject(obj);
    }
    node->detachAllObjects();
    // Now attach the new built terrain mesh.
    node->attachObject(patch.entity);

    patch.patch_geometry_dirty = false;

    ///\todo Regression. Re-enable this to have the EnvironmentEditor module function again.
//    emit HeightmapGeometryUpdated();
}

void EC_Terrain::CreateRootNode()
{
    // If we already have the patch root node, no need to re-create it.
    if (rootNode)
        return;

    OgreRenderer::RendererPtr renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    if (!sceneMgr)
        return;

    rootNode = sceneMgr->createSceneNode();
    sceneMgr->getRootSceneNode()->addChild(rootNode);
}

void EC_Terrain::CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY)
{
    OgreRenderer::RendererPtr renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    if (!sceneMgr || !sceneMgr->getRootSceneNode())
        return;

    if (!rootNode)
        CreateRootNode();

    node = sceneMgr->createSceneNode();
    if (!node)
        return;

    if (rootNode)
        rootNode->addChild(node);
    else // Just as a safety check, if for some odd reason we did not get the root node.
        sceneMgr->getRootSceneNode()->addChild(node);
    
    const float vertexSpacingX = 1.f;
    const float vertexSpacingY = 1.f;
    const float patchSpacingX = 16 * vertexSpacingX;
    const float patchSpacingY = 16 * vertexSpacingY;
    const Ogre::Vector3 patchOrigin(patchX * patchSpacingX, patchY * patchSpacingY, 0.f);

    node->setPosition(patchOrigin);
}

void EC_Terrain::GetTerrainHeightRange(float &minHeight, float &maxHeight)
{
    minHeight = std::numeric_limits<float>::max();
    maxHeight = std::numeric_limits<float>::min();

    for(int i = 0; i < patches.size(); ++i)
        for(int j = 0; j < patches[i].heightData.size(); ++j)
        {
            minHeight = min(minHeight, patches[i].heightData[j]);
            maxHeight = max(maxHeight, patches[i].heightData[j]);
        }
}

void EC_Terrain::DirtyAllTerrainPatches()
{
    for(size_t i = 0; i < patches.size(); ++i)
        patches[i].patch_geometry_dirty = true;
}

void EC_Terrain::RegenerateDirtyTerrainPatches()
{
    PROFILE(EC_Terrain_RegenerateDirtyTerrainPatches);

    for(int y = 0; y < patchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
        {
            EC_Terrain::Patch &scenePatch = GetPatch(x, y);
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
                if (nX >= 0 && nX < patchWidth &&
                    nY >= 0 && nY < patchHeight &&
                    GetPatch(nX, nY).heightData.size() == 0)
                {
                    neighborsLoaded = false;
                    break;
                }
            }

            if (neighborsLoaded)
                GenerateTerrainGeometryForOnePatch(x, y);
        }
}


} 
