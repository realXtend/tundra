// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"

#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_Terrain.h"
#include "CoreException.h"
#include "BinaryAsset.h"
#include "Renderer.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "OgreMaterialUtils.h"
#include "OgreMaterialAsset.h"
#include "LoggingFunctions.h"
#include "TextureAsset.h"
#include "AttributeMetadata.h"
#include "Profiler.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"

#include <Ogre.h>
#include <utility>

#include "MemoryLeakCheck.h"

using namespace std;
using namespace OgreRenderer;

EC_Terrain::EC_Terrain(Scene* scene) :
    IComponent(scene),
    nodeTransformation(this, "Transform"),
    xPatches(this, "Grid Width", 1),
    yPatches(this, "Grid Height", 1),
    material(this, "Material", AssetReference("Ogre Media:RexTerrainPCF.material")),
    heightMap(this, "Heightmap"),
    uScale(this, "Tex. U scale", 0.13f),
    vScale(this, "Tex. V scale", 0.13f),
    patchWidth(1),
    patchHeight(1),
    rootNode(0)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));

    patches.resize(1);
    MakePatchFlat(0, 0, 0.f);

    heightMapAsset = MAKE_SHARED(AssetRefListener);
    connect(heightMapAsset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(TerrainAssetLoaded(AssetPtr)));
}

EC_Terrain::~EC_Terrain()
{
    Destroy();
}

void EC_Terrain::UpdateSignals()
{
    Entity *parent = ParentEntity();
    CreateRootNode();
    if (parent)
    {
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(AttachTerrainRootNode()), Qt::UniqueConnection);
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(AttachTerrainRootNode()), Qt::UniqueConnection); // The Attach function also handles detaches.
    }
}

void EC_Terrain::MakePatchFlat(int x, int y, float heightValue)
{
    Patch &patch = GetPatch(x, y);
    patch.heightData.clear();
    patch.heightData.insert(patch.heightData.end(), cPatchSize*cPatchSize, heightValue);
    patch.patch_geometry_dirty = true;
}

void EC_Terrain::MakeTerrainFlat(float heightValue)
{
    for(int y = 0; y < this->yPatches.Get(); ++y)
        for(int x = 0; x < this->xPatches.Get(); ++x)
            MakePatchFlat(x, y, heightValue);
}

void EC_Terrain::ResizeTerrain(int newPatchWidth, int newPatchHeight)
{
    PROFILE(EC_Terrain_ResizeTerrain);

    const int maxPatchSize = 256;
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
}

void EC_Terrain::AttributesChanged()
{
    bool sizeChanged = xPatches.ValueChanged() || yPatches.ValueChanged();
    bool needFullRecreate = uScale.ValueChanged() || vScale.ValueChanged();
    bool needIncrementalRecreate = needFullRecreate || sizeChanged;

    // If the height map source has changed, we are going to request the new terrain asset,
    // which means that any changes to the current terrain attributes (size, scale, etc.) can
    // be ignored - we will be doing a full reload of the terrain from the asset when it completes loading.
    if (heightMap.ValueChanged())
        sizeChanged = needFullRecreate = needIncrementalRecreate = false;

    if (needFullRecreate)
        DirtyAllTerrainPatches();
    if (sizeChanged)
        ResizeTerrain(xPatches.Get(), yPatches.Get());
    if (needIncrementalRecreate)
        RegenerateDirtyTerrainPatches();
    if (nodeTransformation.ValueChanged())
        UpdateRootNodeTransform();
    if (material.ValueChanged())
    {
        AssetTransferPtr transfer = GetFramework()->Asset()->RequestAsset(material.Get());
        if (transfer)
            connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(MaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    }
    if (heightMap.ValueChanged())
    {
        QString refBody;
        std::map<QString, QString> args = ParseAssetRefArgs(heightMap.Get().ref, &refBody);
        heightMapAsset->HandleAssetRefChange(framework->Asset(), refBody);
    }
}

void EC_Terrain::MaterialAssetLoaded(AssetPtr asset_)
{
    OgreMaterialAsset *ogreMaterial = dynamic_cast<OgreMaterialAsset*>(asset_.get());
    assert(ogreMaterial);
    if (!ogreMaterial)
        return;
    
    ///\todo We can't free here, since something else might be using the material.
//    Ogre::MaterialManager::getSingleton().remove(currentMaterial.toStdString()); // Free up the old material.

    currentMaterial = ogreMaterial->ogreAssetName;

    // Also, we need to update each geometry patch to use the new material.
    for(int y = 0; y < patchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
            UpdateTerrainPatchMaterial(x, y);
}

void EC_Terrain::TerrainAssetLoaded(AssetPtr asset_)
{
    BinaryAssetPtr assetData = dynamic_pointer_cast<BinaryAsset>(asset_);
    TextureAssetPtr textureData = dynamic_pointer_cast<TextureAsset>(asset_);
    if ((!assetData.get() || assetData->data.size() == 0) && !textureData)
    {
        LogError("Failed to load terrain asset from file!");
        return;
    }

    if (assetData)
        LoadFromDataInMemory((const char*)&assetData->data[0], assetData->data.size());

    if (textureData)
    {
        if (textureData->DiskSource().isEmpty())
        {
            LogError("Tried to load terrain from texture file \"" + textureData->Name() + "\", but this requires a disk source of the asset for the load to work properly!");
            return;
        }
        std::map<QString, QString> args = ParseAssetRefArgs(heightMap.Get().ref, 0);
        float scale = 30.f;
        float offset = 0.f;
        if (args.find("scale") != args.end())
            scale = args["scale"].toFloat();
        if (args.find("offset") != args.end())
            offset = args["offset"].toFloat();
        ///\todo Add support for converting the loaded image file directly to a .ntf asset and refer to that one.
        bool success = LoadFromImageFile(textureData->DiskSource(), offset, scale);
        if (!success)
        {
            LogError("Failed to load terrain from texture source \"" + textureData->Name() + "\"! Loading the file \"" + textureData->DiskSource() + "\" failed!");
        }
    }
}

void EC_Terrain::DestroyPatch(int x, int y)
{
    if (x >= patchWidth || y >= patchHeight || x < 0 || y < 0)
        return;

    assert(GetFramework());
    if (!GetFramework())
        return;

    if (world_.expired()) // Oops! Already destroyed
        return;

    Ogre::SceneManager *sceneMgr = world_.lock()->OgreSceneManager();
    
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
        catch(...) {}
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

    if (world_.expired()) // Oops! Already destroyed
        return;
    Ogre::SceneManager *sceneMgr = world_.lock()->OgreSceneManager();

    if (rootNode)
    {
        sceneMgr->destroySceneNode(rootNode);
        rootNode = 0;
    }
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

void EC_Terrain::SetPointHeight(int x, int y, float height)
{
    if (x < 0 || y < 0 || x >= cPatchSize * patchWidth || y >= cPatchSize * patchHeight)
        return; // Out of bounds signals are silently ignored.

    GetPatch(x / cPatchSize, y / cPatchSize).heightData[(y % cPatchSize) * cPatchSize + (x % cPatchSize)] = height;
}

namespace
{
    Ogre::Matrix4 GetWorldTransform(Ogre::SceneNode *node)
    {
        Ogre::Quaternion rot = node->_getDerivedOrientation();
        Ogre::Vector3 trans = node->_getDerivedPosition();
        Ogre::Vector3 scale = node->_getDerivedScale();

        Ogre::Matrix4 worldTM;
        worldTM.makeTransform(trans, scale, rot);

        // In Ogre 1.7.1 we could simply use the following line, but since we're also supporting Ogre 1.6.4 for now, the above
        // lines are used instead, which work in both.
        // Ogre::Matrix4 worldTM = rootNode->_getFullTransform(); // local->world. 

        return worldTM;
    }
}

float3 EC_Terrain::GetPointOnMap(const float3 &point) const 
{
    if (!rootNode)
    {
        LogError("GetPointOnMap called before rootNode initialized, returning zeros");
        return float3(0, 0, 0);
    }
    Ogre::Matrix4 worldTM = GetWorldTransform(rootNode);

    // Note: heightmap X & Y correspond to X & Z world axes, while height is world Y
    Ogre::Matrix4 inv = worldTM.inverse(); // world->local
    Ogre::Vector4 local = inv * Ogre::Vector4(point.x, point.y, point.z, 1.f);
    local.y = GetInterpolatedHeightValue(local.x, local.z);
    Ogre::Vector4 world = worldTM * local;
    return float3(world.x, world.y, world.z);
}

float3 EC_Terrain::GetPointOnMapLocal(const float3 &point) const
{
    if (!rootNode)
    {
        LogError("GetPointOnMapLocal called before rootNode initialized, returning zeros");
        return float3(0, 0, 0);
    }
    Ogre::Matrix4 worldTM = GetWorldTransform(rootNode);

    // Note: heightmap X & Y correspond to X & Z world axes, while height is world Y
    Ogre::Matrix4 inv = worldTM.inverse(); // world->local
    Ogre::Vector4 local = inv * Ogre::Vector4(point.x, point.y, point.z, 1.f);
    local.y = GetInterpolatedHeightValue(local.x, local.z);
    return float3(local.x, local.y, local.z);
}

float EC_Terrain::GetDistanceToTerrain(const float3 &point) const
{
    float3 pointOnMap = GetPointOnMap(point);
    return point.y - pointOnMap.y;
}

bool EC_Terrain::IsOnTopOfMap(const float3 &point) const
{
    return GetDistanceToTerrain(point) >= 0.f;
}

float EC_Terrain::GetInterpolatedHeightValue(float x, float y) const
{
    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    xFloor = Clamp(xFloor, 0, PatchWidth() * cPatchSize - 1);
    xCeil = Clamp(xCeil, 0, PatchWidth() * cPatchSize - 1);
    yFloor = Clamp(yFloor, 0, PatchHeight() * cPatchSize - 1);
    yCeil = Clamp(yCeil, 0, PatchHeight() * cPatchSize - 1);

    float u = fmod(x, 1.f);
    float v = fmod(y, 1.f);
    float h1;
    float h2 = GetPoint(xFloor, yCeil);
    float h3 = GetPoint(xCeil, yFloor);

    if (u + v >= 1.f)
    {
        //if xFrac >= yFrac
        h1 = GetPoint(xCeil, yCeil);
        u = 1.f - u;
        v = 1.f - v;
    }
    else
    {
        h1 = GetPoint(xFloor, yFloor);
        swap(h2, h3);
    }

    return h1 * (1.f - u - v) + h2 * u + h3 * v;
}

float3x4 EC_Terrain::TangentFrame(const float3 &worldPoint) const
{
    float3 pointOnTerrainLocal = GetPointOnMapLocal(worldPoint);

    float3 normal = GetInterpolatedNormal(pointOnTerrainLocal.x, pointOnTerrainLocal.y);
    float3x4 worldTransform = WorldTransform();
    float3 xDir = worldTransform.Col(0); // +X dir of the terrain local map direction runs in this direction in world space.

    // In local tangent space, the +Y axis maps to world-space normal of the terrain at that point.
    float3x4 lookAt = float3x4::LookAt(float3::unitY, normal, float3::unitX, xDir);
    lookAt.SetTranslatePart(GetPointOnMap(worldPoint));
    return lookAt;
}

float3 EC_Terrain::Tangent(const float3 &worldPoint, const float3 &worldDir) const
{
    float3x4 tangentFrame = TangentFrame(worldPoint);
    float3x4 toLocal = tangentFrame;
    toLocal.InverseOrthonormal();
    float3 localDir = toLocal.MulDir(worldDir);
    localDir.y = 0; // Vector in the tangent plane is perpendicular to the surface normal.
    return tangentFrame.MulDir(localDir).Normalized();
}

float3x4 EC_Terrain::WorldTransform() const
{
    Ogre::Matrix4 worldTM = GetWorldTransform(rootNode);
    return float4x4(worldTM).Float3x4Part();
}

void EC_Terrain::GetTriangleNormals(float x, float y, float3 &n1, float3 &n2, float3 &n3, float &u, float &v) const
{
    x = max(0.f, min((float)VerticesWidth()-1.f, x));
    y = max(0.f, min((float)VerticesHeight()-1.f, y));

    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    xFloor = Clamp(xFloor, 0, PatchWidth() * cPatchSize - 1);
    xCeil = Clamp(xCeil, 0, PatchWidth() * cPatchSize - 1);
    yFloor = Clamp(yFloor, 0, PatchHeight() * cPatchSize - 1);
    yCeil = Clamp(yCeil, 0, PatchHeight() * cPatchSize - 1);

    float xFrac = fmod(x, 1.f);
    float yFrac = fmod(y, 1.f);

    n2 = CalculateNormal(static_cast<int>(xFloor), static_cast<int>(yCeil));
    n3 = CalculateNormal(static_cast<int>(xCeil), static_cast<int>(yFloor));

    if (xFrac + yFrac >= 1.f)
    {
        //if xFrac >= yFrac
        n1 = CalculateNormal(static_cast<int>(xCeil), static_cast<int>(yCeil));
        xFrac = 1.f - xFrac;
        yFrac = 1.f - yFrac;
    }
    else
    {
        n1 = CalculateNormal( static_cast<int>(xFloor), static_cast<int>(yFloor));
        swap(n2, n3);
    }
    u = xFrac;
    v = yFrac;
}

void EC_Terrain::GetTriangleVertices(float x, float y, float3 &v1, float3 &v2, float3 &v3, float &u, float &v) const
{
    // Note: heightmap X & Y correspond to X & Z world axes, while height is world Y
    
    x = max(0.f, min((float)VerticesWidth()-1.f, x));
    y = max(0.f, min((float)VerticesHeight()-1.f, y));

    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    xFloor = Clamp(xFloor, 0, PatchWidth() * cPatchSize - 1);
    xCeil = Clamp(xCeil, 0, PatchWidth() * cPatchSize - 1);
    yFloor = Clamp(yFloor, 0, PatchHeight() * cPatchSize - 1);
    yCeil = Clamp(yCeil, 0, PatchHeight() * cPatchSize - 1);

    float xFrac = fmod(x, 1.f);
    float yFrac = fmod(y, 1.f);

    v2 = float3((float)xFloor, GetPoint(xFloor, yCeil), (float)yCeil);
    v3 = float3((float)xCeil, GetPoint(xCeil, yFloor), (float)yFloor);

    if (xFrac + yFrac >= 1.f)
    {
        //if xFrac >= yFrac
        v1 = float3((float)xCeil, GetPoint(xCeil, yCeil), (float)yCeil);
        xFrac = 1.f - xFrac;
        yFrac = 1.f - yFrac;
    }
    else
    {
        v1 = float3((float)xFloor, GetPoint(xFloor, yFloor), (float)yFloor);
        swap(v2, v3);
    }
    u = xFrac;
    v = yFrac;
}

float3 EC_Terrain::GetPlaneNormal(float x, float y) const
{
    float3 h1, h2, h3;
    float u, v;
    GetTriangleVertices(x, y, h1, h2, h3, u, v);

    // h1 to h3 are the three terrain height points in local coordinate space.
    float3 normal = (h3-h2).Cross(h3-h1);

    float4x4 worldTM = GetWorldTransform(rootNode);
    return worldTM.MulDir(normal).Normalized();
}

float3 EC_Terrain::GetInterpolatedNormal(float x, float y) const
{
    float3 n1, n2, n3;
    float u, v;
    GetTriangleNormals(x, y, n1, n2, n3, u, v);

    // h1 to h3 are the three terrain height points in local coordinate space.
    float3 normal = (1.f - u - v) * n1 + u * n2 + v * n3;

    float4x4 worldTM = GetWorldTransform(rootNode);
    return worldTM.MulDir(normal).Normalized();
}

float3 EC_Terrain::CalculateNormal(int x, int y, int xinside, int yinside) const
{
    int px = x * cPatchSize + xinside;
    int py = y * cPatchSize + yinside;

    int xNext = Clamp(px+1, 0, patchWidth * cPatchSize - 1);
    int yNext = Clamp(py+1, 0, patchHeight * cPatchSize - 1);
    int xPrev = Clamp(px-1, 0, patchWidth * cPatchSize - 1);
    int yPrev = Clamp(py-1, 0, patchHeight * cPatchSize - 1);

    float x_slope = GetPoint(xPrev, py) - GetPoint(xNext, py);
    if ((px <= 0) || (px >= patchWidth * cPatchSize))
        x_slope *= 2;
    float y_slope = GetPoint(px, yPrev) - GetPoint(px, yNext);
    if ((py <= 0) || (py >= patchHeight * cPatchSize))
        y_slope *= 2;

    // Note: heightmap X & Y correspond to X & Z world axes, while height is world Y
    return float3(x_slope, 2.0, y_slope).Normalized();
}

bool EC_Terrain::SaveToFile(QString filename)
{
    if (patchWidth * patchHeight != (int)patches.size())
    {
        LogError("The EC_Terrain is in inconsistent state. Cannot save.");
        return false;
    }
    /// @todo Unicode support
    FILE *handle = fopen(filename.toStdString().c_str(), "wb");
    if (!handle)
    {
        LogError("Could not open file " + filename.toStdString() + ".");
        return false;
    }

    const u32 xPatches = patchWidth;
    const u32 yPatches = patchHeight;
    fwrite(&xPatches, sizeof(u32), 1, handle);
    fwrite(&yPatches, sizeof(u32), 1, handle);

    assert(sizeof(float) == 4);

    for(u32 i = 0; i < xPatches*yPatches; ++i)
    {
        if (patches[i].heightData.size() < cPatchSize*cPatchSize)
            patches[i].heightData.resize(cPatchSize*cPatchSize);

        fwrite(&patches[i].heightData[0], sizeof(float), cPatchSize*cPatchSize, handle); ///< \todo Check read error.
    }
    fflush(handle);
    if (ferror(handle))
    LogError("Write error in SaveToFile");
    fclose(handle);

    return true;
}

u32 ReadU32(const char *dataPtr, size_t numBytes, int &offset)
{
    if (offset + 4 > (int)numBytes)
        throw Exception("Not enough bytes to deserialize!");
    u32 data = *(u32*)(dataPtr + offset); ///@note Requires unaligned load support from the CPU and assumes data storage endianness to be the same for loader and saver.
    offset += 4;
    return data;
}

bool EC_Terrain::LoadFromFile(QString filename)
{
    filename = filename.trimmed();

    std::vector<u8> file;
    LoadFileToVector(filename, file);

    if (file.size() > 0)
    {
        bool success = LoadFromDataInMemory((const char *)&file[0], file.size());
        if (success)
            currentHeightmapAssetSource = filename;
        return success;
    }
    return false;
}

bool EC_Terrain::LoadFromDataInMemory(const char *data, size_t numBytes)
{
    int offset = 0;
    u32 xPatches = ReadU32(data, numBytes, offset);
    u32 yPatches = ReadU32(data, numBytes, offset);

    // Load all the data from the file to an intermediate buffer first, so that we can first see
    // if the file is not broken, and reject it without losing the old terrain.
    std::vector<Patch> newPatches(xPatches*yPatches);

    // Initialize the new height data structure.
    for(u32 y = 0; y < yPatches; ++y)
        for(u32 x = 0; x < xPatches; ++x)
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
        if (offset+cPatchSize*cPatchSize*sizeof(float) > numBytes)
            throw Exception("Not enough bytes to deserialize!");

        memcpy(&newPatches[i].heightData[0], data + offset, cPatchSize*cPatchSize*sizeof(float));
        offset += cPatchSize*cPatchSize*sizeof(float);
    }

    // The terrain asset loaded ok. We are good to set that terrain as the active terrain.
    Destroy();

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

    return true;
}

void EC_Terrain::NormalizeImage(QString filename) const
{
    Ogre::Image image;
    try
    {
        std::vector<u8> imageFile;
        LoadFileToVector(filename, imageFile);
#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&imageFile[0], imageFile.size(), false));
#include "EnableMemoryLeakCheck.h"
        image.load(stream);
    }
    catch(...)
    {
        LogError("EC_Terrain::NormalizeImage: Exception catched when trying load image file" + filename + ".");
        return;
    }

    std::vector<uchar> imageData(4*image.getWidth()*image.getHeight(), 0);

    uchar *imagePos = &imageData[0];

    for(size_t y = 0; y < image.getHeight(); ++y)
        for(size_t x = 0; x < image.getWidth(); ++x)
        {
            Ogre::ColourValue color = image.getColourAt(x, y, 0);
            color.a = 0;
            color /= (color.r + color.g + color.b + color.a);
            *imagePos++ = (uchar)Clamp((int)(color.b * 255), 0, 255);
            *imagePos++ = (uchar)Clamp((int)(color.g * 255), 0, 255);
            *imagePos++ = (uchar)Clamp((int)(color.r * 255), 0, 255);
            *imagePos++ = (uchar)Clamp((int)(color.a * 255), 0, 255);
        }
    
    try
    {
        Ogre::Image dstImage;
        dstImage.loadDynamicImage(&imageData[0], image.getWidth(), image.getHeight(), Ogre::PF_A8R8G8B8);
        dstImage.save(filename.toStdString());
    }
    catch(...)
    {
        LogWarning("Exception catched when trying save image file" + filename + ".");
    }
}

bool EC_Terrain::LoadFromImageFile(QString filename, float offset, float scale)
{
    Ogre::Image image;
    try
    {
        std::vector<u8> imageFile;
        LoadFileToVector(filename, imageFile);
#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&imageFile[0], imageFile.size(), false));
#include "EnableMemoryLeakCheck.h"
        image.load(stream);
    }
    catch(...)
    {
        LogError("Execption catched when trying load terrain from image file" + filename + ".");
        return false;
    }

    // Note: In the following, we round down, so if the image size is not a multiple of cPatchSize (== 16),
    // we will not use the whole image contents.
    xPatches.Set(image.getWidth() / cPatchSize, AttributeChange::Disconnected);
    yPatches.Set(image.getHeight() / cPatchSize, AttributeChange::Disconnected);
    ResizeTerrain(xPatches.Get(), yPatches.Get());

    for(int y = 0; y < yPatches.Get() * cPatchSize; ++y)
        for(int x = 0; x < xPatches.Get() * cPatchSize; ++x)
        {
            Ogre::ColourValue c = image.getColourAt(x, y, 0);
            float height = offset + scale * (c.r + c.g + c.b) / 3.f; // Treat the image as a grayscale heightmap field with the color in range [0,1].
            SetPointHeight(x, y, height);
        }

    xPatches.Changed(AttributeChange::LocalOnly);
    yPatches.Changed(AttributeChange::LocalOnly);

    DirtyAllTerrainPatches();
    RegenerateDirtyTerrainPatches();

    return true;
}

bool EC_Terrain::SaveToImageFile(QString filename, float minHeight, float maxHeight)
{
    const int xVertices = xPatches.Get() * cPatchSize;
    const int yVertices = yPatches.Get() * cPatchSize;

    if (minHeight <= -1e8f)
        minHeight = GetTerrainMinHeight();
    if (maxHeight >= 1e8f)
        maxHeight = GetTerrainMaxHeight();
    
    std::vector<uchar> imageData(3*xVertices*yVertices, 0);

    if (maxHeight - minHeight > 1e-5f) // If the terrain is not flat, read the actual values. If the terrain is flat, a black image is outputted.
        for(int y = 0; y < yPatches.Get() * cPatchSize; ++y)
            for(int x = 0; x < xPatches.Get() * cPatchSize; ++x)
            {
                float height = 255.f * (GetPoint(x, y) - minHeight) / (maxHeight - minHeight);
                int h = min(255, max((int)height, 0));
                imageData[(y*xVertices+x)*3] = imageData[(y*xVertices+x)*3+1] = imageData[(y*xVertices+x)*3+2] = (uchar)h;
            }
    
    try
    {
        Ogre::Image image;
        image.loadDynamicImage(&imageData[0], xVertices, yVertices, Ogre::PF_R8G8B8);
        image.save(filename.toStdString().c_str());
    }
    catch(...)
    {
        LogWarning("EC_Terrain::SaveToImageFile: Exception catched when trying save image file" + filename + ".");
        return false;
    }
    return true;
}

/// An utility function that extracts geometry vertex and index data from an Ogre mesh. See Renderer.cpp, GetMeshInformation for the origin of this function.
void GetUnskinnedMeshGeometry(
    Ogre::Mesh *mesh, // The mesh to read the vertex and index data from.
    std::vector<Ogre::Vector3>& vertices, // [out] The geometry vertex data.
    std::vector<uint>& indices, // [out] The geometry index data.
    std::vector<uint>& submeshstartindex) // [out] Points to the index buffer, specifies the range starts for distinct submeshes.
{
    PROFILE(Renderer_GetUnskinnedMeshGeometry);

    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;
    size_t vertex_count = 0;
    size_t index_count = 0;

    assert(mesh);

    submeshstartindex.resize(mesh->getNumSubMeshes());

    // Calculate how many vertices and indices we're going to need
    for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh( i );
        // We only need to add the shared vertices once
        if (submesh->useSharedVertices)
        {
            if (!added_shared)
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }

        // Add the indices
        submeshstartindex[i] = index_count;
        index_count += submesh->indexData->indexCount;
    }

    // Allocate space for the vertices and indices
    vertices.resize(vertex_count);
    indices.resize(index_count);

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

        // Get vertex data
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if ((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal = 0;

            for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

                vertices[current_offset + j] = pt;
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }

        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        u32*  pLong = static_cast<u32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        u16* pShort = reinterpret_cast<u16*>(pLong);
        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        if (use32bitindexes)
            for(size_t k = 0; k < numTris*3; ++k)
                indices[index_offset++] = pLong[k] + static_cast<uint>(offset);
        else
            for(size_t k = 0; k < numTris*3; ++k)
                indices[index_offset++] = static_cast<uint>(pShort[k]) + static_cast<unsigned long>(offset);

        ibuf->unlock();
        current_offset = next_offset;
    }
}

float FindClosestRayIntersection(const Ogre::Ray &ray, const std::vector<Ogre::Vector3> &vertices, const std::vector<uint> &indices)
{
    using namespace std;

    float distance = 1e9f;
    for(size_t i = 0; i+2 < indices.size(); i += 3)
    {
        std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[i]],
            vertices[indices[i+1]], vertices[indices[i+2]], true, true);
        if (hit.first && hit.second >= 0.f)
            distance = min(distance, hit.second);
    }
    return distance;
}

void ComputeAABB(const std::vector<Ogre::Vector3> &vertices, Ogre::Vector3 &minExtents, Ogre::Vector3 &maxExtents)
{
    minExtents = Ogre::Vector3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    maxExtents = Ogre::Vector3(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    for(size_t i = 0; i < vertices.size(); ++i)
    {
        minExtents.x = min(minExtents.x, vertices[i].x);
        minExtents.y = min(minExtents.y, vertices[i].y);
        minExtents.z = min(minExtents.z, vertices[i].z);

        maxExtents.x = max(maxExtents.x, vertices[i].x);
        maxExtents.y = max(maxExtents.y, vertices[i].y);
        maxExtents.z = max(maxExtents.z, vertices[i].z);
    }
}

void EC_Terrain::GenerateFromSceneEntity(QString entityName)
{
    Entity *parentEntity = ParentEntity();
    if (!parentEntity)
        return;

    Scene *scene = parentEntity->ParentScene();
    if (!scene)
        return;

    Entity *entity = scene->GetEntityByName(entityName).get();
    if (!entity)
        return;

    EC_Placeable *position = entity->GetComponent<EC_Placeable>().get();
    if (!position)
        return;

    EC_Mesh *mesh = entity->GetComponent<EC_Mesh>().get();
    if (!mesh)
        return;

    Ogre::Entity *ogreEntity = mesh->GetEntity();
    if (!ogreEntity)
        return;

    Ogre::SceneNode *ogreNode = ogreEntity->getParentSceneNode();
    if (!ogreNode)
        return;

    std::string ogreMeshName = mesh->GetMeshName();

    Ogre::Quaternion rot = ogreNode->_getDerivedOrientation();
    Ogre::Vector3 trans = ogreNode->_getDerivedPosition();
    Ogre::Vector3 scale = ogreNode->_getDerivedScale();

    Ogre::Matrix4 worldTM;
    worldTM.makeTransform(trans, scale, rot);

    GenerateFromOgreMesh(ogreMeshName.c_str(), worldTM);
}

void EC_Terrain::GenerateFromOgreMesh(QString ogreMeshResourceName)
{    
    GenerateFromOgreMesh(ogreMeshResourceName, Ogre::Matrix4::IDENTITY);
}

void EC_Terrain::GenerateFromOgreMesh(QString ogreMeshResourceName, const Ogre::Matrix4 &transform)
{
    using namespace std;

    Ogre::Mesh *mesh = dynamic_cast<Ogre::Mesh*>(Ogre::MeshManager::getSingleton().getByName(ogreMeshResourceName.toStdString().c_str()).get());
    if (!mesh)
    {
        LogError("Could not get mesh " + ogreMeshResourceName + ".");
        return;
    }

    std::vector<Ogre::Vector3> vertices;
    std::vector<uint> indices;
    std::vector<uint> submeshstartindex;

    GetUnskinnedMeshGeometry(mesh, vertices, indices, submeshstartindex);

    for(size_t i = 0; i < vertices.size(); ++i)
    {
        Ogre::Vector4 v = transform * Ogre::Vector4(vertices[i].x, vertices[i].y, vertices[i].z, 1.f);
        vertices[i] = Ogre::Vector3(v.x, v.y, v.z);
    }

    Ogre::Vector3 minExtents;
    Ogre::Vector3 maxExtents;
    ComputeAABB(vertices, minExtents, maxExtents);

    // Note: heightmap X & Y correspond to X & Z world axes, while height is world Y.
    // So we expect a mesh where Y also represent height values
    int xVertices = (int)ceil(maxExtents.x - minExtents.x);
    int yVertices = (int)ceil(maxExtents.z - minExtents.z);
    xVertices = ((xVertices + cPatchSize-1) / cPatchSize) * cPatchSize;
    yVertices = ((yVertices + cPatchSize-1) / cPatchSize) * cPatchSize;

    xPatches.Set(xVertices/cPatchSize, AttributeChange::Disconnected);
    yPatches.Set(yVertices/cPatchSize, AttributeChange::Disconnected);
    ResizeTerrain(xVertices/cPatchSize, yVertices/cPatchSize);

    MakeTerrainFlat(1e9f);

    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = -std::numeric_limits<float>::max();

    const float raycastHeight = maxExtents.y + 100.f;

    for(int y = 0; y < yVertices; ++y)
        for(int x = 0; x < xVertices; ++x)
        {
            Ogre::Ray r(Ogre::Vector3(minExtents.x + x, raycastHeight, minExtents.z + y), Ogre::Vector3(0,-1.f,0));
            float height = FindClosestRayIntersection(r, vertices, indices);
            if (height < 1e8f)
            {
                height = raycastHeight - height;
                SetPointHeight(x, y, height);
                minHeight = min(minHeight, height);
                maxHeight = max(maxHeight, height);
            }
        }

    for(int y = 0; y < yVertices; ++y)
        for(int x = 0; x < xVertices; ++x)
            if (GetPoint(x, y) >= 1e8f)
                SetPointHeight(x, y, minHeight);

    // Adjust offset so that we always have the lowest point of the terrain at height 0.
    RemapHeightValues(0.f, maxHeight - minHeight);

    heightMap.Set(AssetReference(""/*,""*/), AttributeChange::Disconnected);

    xPatches.Changed(AttributeChange::LocalOnly);
    yPatches.Changed(AttributeChange::LocalOnly);
    heightMap.Changed(AttributeChange::LocalOnly);

    DirtyAllTerrainPatches();
    RegenerateDirtyTerrainPatches();
}

void EC_Terrain::AffineTransform(float scale, float offset)
{
    for(int y = 0; y < yPatches.Get() * cPatchSize; ++y)
        for(int x = 0; x < xPatches.Get() * cPatchSize; ++x)
            SetPointHeight(x, y, GetPoint(x, y) * scale + offset);
}

void EC_Terrain::RemapHeightValues(float minHeight, float maxHeight)
{
    float minHeightCur = 1e9f;
    float maxHeightCur = -1e9f;

    for(int y = 0; y < yPatches.Get() * cPatchSize; ++y)
        for(int x = 0; x < xPatches.Get() * cPatchSize; ++x)
        {
            minHeightCur = min(minHeightCur, GetPoint(x, y));
            maxHeightCur = max(maxHeightCur, GetPoint(x, y));
        }

    // There is no height variance in the current terrain height values. Make the whole terrain show the minHeight value.
    if (fabs(maxHeightCur - minHeightCur) < 1e-4f)
    {
        MakeTerrainFlat(minHeight);
        return;
    }

    AffineTransform((maxHeight - minHeight) / (maxHeightCur - minHeightCur), minHeight - minHeightCur);
}

void EC_Terrain::SetTerrainMaterialTexture(int index, const QString &textureName)
{
    if (index < 0 || index > 4)
        return;

//    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
//    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(textureName).get());

    /// @bug This performs a lossy Unicode->ASCII conversion for the material name!
    Ogre::MaterialPtr terrainMaterial = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
    if (!terrainMaterial.get())
    {
//        LogWarning("Ogre material " + material.Get() + " not found!");
        return;
    }
//    Ogre::MaterialPtr terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial(terrainMaterialName);
//    assert(terrainMaterial);
 //   if(terrainMaterial)
//    {
    /// @bug This performs a lossy Unicode->ASCII conversion for the texture name!
        OgreRenderer::SetTextureUnitOnMaterial(terrainMaterial, textureName.toStdString().c_str(), index);
//    }
//    else
//        LogWarning("Ogre material " + std::string(terrainMaterialName) + " not found!");
}

void EC_Terrain::UpdateTerrainPatchMaterial(int patchX, int patchY)
{
    Patch &patch = GetPatch(patchX, patchY);

    if (!patch.entity)
        return;

    for(size_t i = 0; i < patch.entity->getNumSubEntities(); ++i)
    {
        Ogre::SubEntity *sub = patch.entity->getSubEntity(i);
        if (sub)
            sub->setMaterialName(currentMaterial.toStdString().c_str());
    }
}

void EC_Terrain::UpdateRootNodeTransform()
{
    if (!rootNode)
        return;

    const Transform &tm = nodeTransformation.Get();

    Ogre::Matrix3 rot_new;
    rot_new.FromEulerAnglesXYZ(Ogre::Degree(tm.rot.x), Ogre::Degree(tm.rot.y), Ogre::Degree(tm.rot.z));
    Ogre::Quaternion q_new(rot_new);

    rootNode->setOrientation(Ogre::Quaternion(rot_new));
    rootNode->setPosition(tm.pos.x, tm.pos.y, tm.pos.z);
    rootNode->setScale(tm.scale.x, tm.scale.y, tm.scale.z);
}

void EC_Terrain::AttachTerrainRootNode()
{
    if (!rootNode)
        CreateRootNode();

    if (world_.expired()) 
        return;
    Ogre::SceneManager *sceneMgr = world_.lock()->OgreSceneManager();

    // Detach the terrain root node from any previous EC_Placeable scenenode.
    if (rootNode->getParentSceneNode())
        rootNode->getParentSceneNode()->removeChild(rootNode);

    // If this entity has an EC_Placeable, make sure it is the parent of this terrain component.
    shared_ptr<EC_Placeable> pos = ParentEntity()->GetComponent<EC_Placeable>();
    if (pos)
    {
        Ogre::SceneNode *parent = pos->GetSceneNode();
        parent->addChild(rootNode);
        rootNode->setVisible(pos->visible.Get()); // Re-apply visibility on all the geometry.
    }
    else
    {
        // No EC_Placeable: the root node is attached to the scene directly.
        sceneMgr->getRootSceneNode()->addChild(rootNode);
    }
}

void EC_Terrain::GenerateTerrainGeometryForOnePatch(int patchX, int patchY)
{
    PROFILE(EC_Terrain_GenerateTerrainGeometryForOnePatch);

    EC_Terrain::Patch &patch = GetPatch(patchX, patchY);

    if (!ViewEnabled())
        return;
    if (world_.expired())
        return;
    OgreWorldPtr world = world_.lock();
    Ogre::SceneManager *sceneMgr = world->OgreSceneManager();

    Ogre::SceneNode *node = patch.node;
    bool firstTimeFill = (node == 0);
    UNREFERENCED_PARAM(firstTimeFill);
    if (!node)
    {
        CreateOgreTerrainPatchNode(node, patch.x, patch.y);
        patch.node = node;
    }
    assert(node);

    Ogre::MaterialPtr terrainMaterial = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
    if (!terrainMaterial.get()) // If we could not find the material we were supposed to use, just use the default system terrain material.
        terrainMaterial = OgreRenderer::GetOrCreateLitTexturedMaterial("Rex/TerrainPCF");

    Ogre::ManualObject *manual = sceneMgr->createManualObject(world->GetUniqueObjectName("EC_Terrain_manual"));
    manual->setCastShadows(false);

    manual->clear();
    manual->estimateVertexCount((cPatchSize+1)*(cPatchSize+1));
    manual->estimateIndexCount((cPatchSize+1)*(cPatchSize+1)*3*2);
    manual->begin(terrainMaterial->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

    const float vertexSpacingX = 1.f;
    const float vertexSpacingY = 1.f;
    const float patchSpacingX = cPatchSize * vertexSpacingX;
    const float patchSpacingY = cPatchSize * vertexSpacingY;
    const Ogre::Vector3 patchOrigin(patch.x * patchSpacingX, 0.f, patch.y * patchSpacingY);
// Opensim:    const Ogre::Vector3 patchOrigin(patch.x * patchSpacingX, patch.y * patchSpacingY, 0.f);

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
            pos.x = vertexSpacingX * x;
            pos.z = vertexSpacingY * y;
// Opensim:             pos.x = vertexSpacingX * x;
// Opensim              pos.y = vertexSpacingY * y;

            EC_Terrain::Patch *thisPatch;
            int X = x;
            int Y = y;
            if (x < cPatchVertexWidth && y < cPatchVertexHeight)
            {
                thisPatch = &patch;

                if ((patch.x + 1 < patchWidth || x+1 < cPatchVertexWidth) &&
                    (patch.y + 1 < patchHeight || y+1 < cPatchVertexHeight))
                {
                    // Note: winding needs to be flipped when terrain X axis goes along world X axis and terrain Y axis along world Z
                    manual->index(curIndex+stride);
                    manual->index(curIndex+1);
                    manual->index(curIndex);

                    manual->index(curIndex+stride);
                    manual->index(curIndex+stride+1);
                    manual->index(curIndex+1);
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

            pos.y = thisPatch->heightData[Y*cPatchVertexWidth+X];
// Opensim:            pos.z = thisPatch->heightData[Y*cPatchVertexWidth+X];

            manual->position(pos);
            manual->normal(CalculateNormal(thisPatch->x, thisPatch->y, X, Y));

            // The UV set 0 contains the diffuse texture UV map. Do a planar mapping with the given specified UV scale.
            manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.z + pos.z) * vScale);
// Opensim:             manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.y + pos.y) * vScale);

            // The UV set 1 contains the terrain blend mask UV map, which stretches once across the whole terrain.
            manual->textureCoord((float)(patch.x*cPatchSize + x)/(VerticesWidth()-1), (float)(patch.y*cPatchSize + y)/(VerticesHeight()-1)); 

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
        catch(...) {}
    }

    patch.meshGeometryName = world->GetUniqueObjectName("EC_Terrain_patchmesh");
    Ogre::MeshPtr terrainMesh = manual->convertToMesh(patch.meshGeometryName);

    // Odd: destroyManualObject seems to leave behind a memory leak if we don't call manualObject->clear first.
    manual->clear();
    sceneMgr->destroyManualObject(manual);

    patch.entity = sceneMgr->createEntity(world->GetUniqueObjectName("EC_Terrain_patchentity"), patch.meshGeometryName);
    patch.entity->setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
    patch.entity->setCastShadows(false);
    // Set UserAny also on subentities
    for(uint i = 0; i < patch.entity->getNumSubEntities(); ++i)
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
}

void EC_Terrain::CreateRootNode()
{
    // If we already have the patch root node, no need to re-create it.
    if (rootNode)
        return;

    if (world_.expired())
        return;
    OgreWorldPtr world = world_.lock();
    Ogre::SceneManager *sceneMgr = world->OgreSceneManager();

    rootNode = sceneMgr->createSceneNode(world->GetUniqueObjectName("EC_Terrain_RootNode"));

    // Add the newly created node to scene or to a parent EC_Placeable.
    AttachTerrainRootNode();

    UpdateRootNodeTransform();
}

void EC_Terrain::CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY)
{
    if (world_.expired())
        return;
    OgreWorldPtr world = world_.lock();
    Ogre::SceneManager *sceneMgr = world->OgreSceneManager();
    
    if (!sceneMgr || !sceneMgr->getRootSceneNode())
        return;

    if (!rootNode)
        CreateRootNode();

    QString name = QString("EC_Terrain_Patch_") + QString::number(patchX) + "_" + QString::number(patchY);
    node = sceneMgr->createSceneNode(world->GetUniqueObjectName(name.toStdString()));
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
    const Ogre::Vector3 patchOrigin(patchX * patchSpacingX, 0.f, patchY * patchSpacingY);

    node->setPosition(patchOrigin);
}

float EC_Terrain::GetTerrainMinHeight() const
{
    float minHeight = std::numeric_limits<float>::max();

    for(size_t i = 0; i < patches.size(); ++i)
        for(size_t j = 0; j < patches[i].heightData.size(); ++j)
            minHeight = min(minHeight, patches[i].heightData[j]);

    return minHeight;
}

float EC_Terrain::GetTerrainMaxHeight() const
{
    float maxHeight = -std::numeric_limits<float>::max();

    for(size_t i = 0; i < patches.size(); ++i)
        for(size_t j = 0; j < patches[i].heightData.size(); ++j)
            maxHeight = max(maxHeight, patches[i].heightData[j]);

    return maxHeight;
}

void EC_Terrain::Resize(int newWidth, int newHeight, int oldPatchStartX, int oldPatchStartY)
{
    std::vector<Patch> newPatches(newWidth * newHeight);
    for(int y = 0; y < newHeight && y + oldPatchStartY < yPatches.Get(); ++y)
        for(int x = 0; x < newWidth && x + oldPatchStartX < xPatches.Get(); ++x)
            newPatches[y * newWidth + x] = patches[(y + oldPatchStartY) * xPatches.Get() + x + oldPatchStartX];

    patches = newPatches;
    xPatches.Set(newWidth, AttributeChange::Disconnected);
    yPatches.Set(newHeight, AttributeChange::Disconnected);
    patchWidth = newWidth;
    patchHeight = newHeight;
    DirtyAllTerrainPatches();
    RegenerateDirtyTerrainPatches();
}

void EC_Terrain::GetTerrainHeightRange(float &minHeight, float &maxHeight) const
{
    minHeight = GetTerrainMinHeight();
    maxHeight = GetTerrainMaxHeight();
}

void EC_Terrain::DirtyAllTerrainPatches()
{
    for(size_t i = 0; i < patches.size(); ++i)
        patches[i].patch_geometry_dirty = true;
}

void EC_Terrain::RegenerateDirtyTerrainPatches()
{
    PROFILE(EC_Terrain_RegenerateDirtyTerrainPatches);

    Entity *parentEntity = ParentEntity();
    if (!parentEntity)
        return;
    EC_Placeable *position = parentEntity->GetComponent<EC_Placeable>().get();
    if (!GetFramework()->IsHeadless() && (!position || position->visible.Get())) // Only need to create GPU resources if the placeable itself is visible.
    {
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
    
    // All the new geometry we created will be visible for Ogre by default. If the EC_Placeable's visible attribute is false,
    // we need to hide all newly created geometry.
    AttachTerrainRootNode();

    emit TerrainRegenerated();
}
