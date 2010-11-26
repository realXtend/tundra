// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Terrain.h"

#include "Renderer.h"
#include "IModule.h"
#include "ServiceManager.h"
#include "Entity.h"
#include "SceneManager.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "OgreMaterialUtils.h"
#include "OgreMaterialResource.h"
#include "OgreTextureResource.h"
#include "OgreConversionUtils.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Terrain")

#include <Ogre.h>
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

    xPatches.Set(1, AttributeChange::Disconnected);
    yPatches.Set(1, AttributeChange::Disconnected);
    patches.resize(1);
    MakePatchFlat(0, 0, 0.f);
    uScale.Set(0.13f, AttributeChange::Disconnected);
    vScale.Set(0.13f, AttributeChange::Disconnected);
    texture0.Set(AssetReference(""), AttributeChange::Disconnected);
    texture1.Set(AssetReference(""), AttributeChange::Disconnected);
    texture2.Set(AssetReference(""), AttributeChange::Disconnected);
    texture3.Set(AssetReference(""), AttributeChange::Disconnected);
    texture4.Set(AssetReference(""), AttributeChange::Disconnected);
    material.Set(AssetReference("file://RexTerrainPCF.material"), AttributeChange::Disconnected);

    heightMapAsset = boost::shared_ptr<AssetRefListener>(new AssetRefListener);
    connect(heightMapAsset.get(), SIGNAL(Downloaded(IAssetTransfer*)), this, SLOT(TerrainAssetLoaded(IAssetTransfer *)));
}

EC_Terrain::~EC_Terrain()
{
    Destroy();
}

void EC_Terrain::UpdateSignals()
{
    disconnect(this, SLOT(AttributeUpdated(IAttribute *)));

    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
        this, SLOT(AttributeUpdated(IAttribute*)), Qt::UniqueConnection);

    Scene::Entity *parent = GetParentEntity();
    if (parent)
    {    
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(AttachTerrainRootNode()), Qt::UniqueConnection);
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(AttachTerrainRootNode()), Qt::UniqueConnection);
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
}

//! Emitted when some of the attributes has been changed.
void EC_Terrain::AttributeUpdated(IAttribute *attribute)
{
    std::string changedAttribute = attribute->GetNameString();

    if (changedAttribute == xPatches.GetNameString() || changedAttribute == yPatches.GetNameString())
    {
        ResizeTerrain(xPatches.Get(), yPatches.Get());
        // Re-do all the geometry on the GPU.
        RegenerateDirtyTerrainPatches();
    }
    else if (changedAttribute == nodeTransformation.GetNameString())
    {
        UpdateRootNodeTransform();
    }
    else if (changedAttribute == material.GetNameString())
    {
        // Request the new material resource. Once it has loaded, MaterialAssetLoaded will be called.
        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(material.Get());
        connect(transfer, SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(MaterialAssetLoaded(IAssetTransfer*)), Qt::UniqueConnection);
    }
    else if (changedAttribute == texture0.GetNameString())
    {
        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(texture0.Get());
        connect(transfer, SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(TextureAssetLoaded(IAssetTransfer*)), Qt::UniqueConnection);
    }
    else if (changedAttribute == texture1.GetNameString())
    {
        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(texture1.Get());
        connect(transfer, SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(TextureAssetLoaded(IAssetTransfer*)), Qt::UniqueConnection);
    }
    else if (changedAttribute == texture2.GetNameString())
    {
        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(texture2.Get());
        connect(transfer, SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(TextureAssetLoaded(IAssetTransfer*)), Qt::UniqueConnection);
    }
    else if (changedAttribute == texture3.GetNameString())
    {
        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(texture3.Get());
        connect(transfer, SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(TextureAssetLoaded(IAssetTransfer*)), Qt::UniqueConnection);
    }
    else if (changedAttribute == texture4.GetNameString())
    {
        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(texture4.Get());
        connect(transfer, SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(TextureAssetLoaded(IAssetTransfer*)), Qt::UniqueConnection);
    }
    else if (changedAttribute == heightMap.GetNameString())
    {
        heightMapAsset->HandleAssetRefChange(attribute);
//        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(AssetReference(heightMap.Get().ref/*, "Terrain"*/));
//        connect(transfer, SIGNAL(Downloaded(IAssetTransfer*)), this, SLOT(TerrainAssetLoaded()), Qt::UniqueConnection);
    }
    else if (changedAttribute == uScale.GetNameString() || changedAttribute == vScale.GetNameString())
    {
        // Re-do all the geometry on the GPU.
        DirtyAllTerrainPatches();
        RegenerateDirtyTerrainPatches();
    }

    ///\todo Delete the old unused textures.
}

void EC_Terrain::MaterialAssetLoaded(IAssetTransfer *transfer)
{
    assert(transfer);
    if (!transfer)
        return;

    OgreRenderer::OgreMaterialResource *ogreMaterial = dynamic_cast<OgreRenderer::OgreMaterialResource *>(transfer->resourcePtr.get());
    assert(ogreMaterial);
    if (!ogreMaterial)
        return;

    Ogre::MaterialPtr material = ogreMaterial->GetMaterial();
    
    ///\todo We can't free here, since something else might be using the material.
//    Ogre::MaterialManager::getSingleton().remove(currentMaterial.toStdString()); // Free up the old material.

    currentMaterial = material->getName().c_str();

    // Also, we need to update each geometry patch to use the new material.
    for(int y = 0; y < patchHeight; ++y)
        for(int x = 0; x < patchWidth; ++x)
            UpdateTerrainPatchMaterial(x, y);
/*
    // The material of the terrain has changed. Since we specify the textures of that material as attributes,
    // we need to re-apply the textures from the attributes to the new material we set.
    SetTerrainMaterialTexture(0, texture0.Get().ref.toStdString().c_str());
    SetTerrainMaterialTexture(1, texture1.Get().ref.toStdString().c_str());
    SetTerrainMaterialTexture(2, texture2.Get().ref.toStdString().c_str());
    SetTerrainMaterialTexture(3, texture3.Get().ref.toStdString().c_str());
    SetTerrainMaterialTexture(4, texture4.Get().ref.toStdString().c_str());
*/
}

void EC_Terrain::TextureAssetLoaded(IAssetTransfer *transfer)
{
    assert(transfer);
    if (!transfer)
        return;

    OgreRenderer::OgreTextureResource *ogreTexture = dynamic_cast<OgreRenderer::OgreTextureResource *>(transfer->resourcePtr.get());
    assert(ogreTexture);
    if (!ogreTexture)
        return;

    Ogre::TexturePtr texture = ogreTexture->GetTexture();
    
    if (transfer->source == texture0.Get()) SetTerrainMaterialTexture(0, texture->getName().c_str());
    else if (transfer->source == texture1.Get()) SetTerrainMaterialTexture(1, texture->getName().c_str());
    else if (transfer->source == texture2.Get()) SetTerrainMaterialTexture(2, texture->getName().c_str());
    else if (transfer->source == texture3.Get()) SetTerrainMaterialTexture(3, texture->getName().c_str());
    else if (transfer->source == texture4.Get()) SetTerrainMaterialTexture(4, texture->getName().c_str());
}

void EC_Terrain::TerrainAssetLoaded(IAssetTransfer *transfer)
{
    assert(transfer);
    if (!transfer || !transfer->assetPtr)
        return;

    LoadFromDataInMemory((const char*)transfer->assetPtr->GetData(), transfer->assetPtr->GetSize());    
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

Vector3df EC_Terrain::GetPointOnMap(const Vector3df &point) const 
{
    Ogre::Matrix4 worldTM = GetWorldTransform(rootNode);

    Ogre::Matrix4 inv = worldTM.inverse(); // world->local
    Ogre::Vector4 local = inv * Ogre::Vector4(point.x, point.y, point.z, 1.f);
    local.z = GetInterpolatedHeightValue(local.x, local.y);
    Ogre::Vector4 world = worldTM * local;
    return Vector3df(world.x, world.y, world.z);
}

Vector3df EC_Terrain::GetPointOnMapLocal(const Vector3df &point) const
{
    Ogre::Matrix4 worldTM = GetWorldTransform(rootNode);

    Ogre::Matrix4 inv = worldTM.inverse(); // world->local
    Ogre::Vector4 local = inv * Ogre::Vector4(point.x, point.y, point.z, 1.f);
    local.z = GetInterpolatedHeightValue(local.x, local.y);
    return Vector3df(local.x, local.y, local.z);
}

float EC_Terrain::GetDistanceToTerrain(const Vector3df &point) const
{
    Vector3df pointOnMap = GetPointOnMap(point);
    return point.z - pointOnMap.z;
}

bool EC_Terrain::IsOnTopOfMap(const Vector3df &point) const
{
    return GetDistanceToTerrain(point) >= 0.f;
}

float EC_Terrain::GetInterpolatedHeightValue(float x, float y) const
{
    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    xFloor = clamp(xFloor, 0, PatchWidth() * cPatchSize - 1);
    xCeil = clamp(xCeil, 0, PatchWidth() * cPatchSize - 1);
    yFloor = clamp(yFloor, 0, PatchHeight() * cPatchSize - 1);
    yCeil = clamp(yCeil, 0, PatchHeight() * cPatchSize - 1);

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

Vector3df EC_Terrain::GetTerrainRotationAngles(float x, float y, float z, const Vector3df& direction) const
{
    Vector3df worldPos(x,y,z);
    Vector3df local = GetPointOnMapLocal(worldPos);
    // Get terrain normal.
    Vector3df worldUp = GetInterpolatedNormal(local.x,local.y);

    // Get a vector which is perpendicular for direction and plane normal
    Vector3df xVec = direction.crossProduct(worldUp);
    Vector3df front = worldUp.crossProduct(xVec);
    
    xVec.normalize();  // X 
    front.normalize(); // Y 
    xVec = -xVec;
    front = -front;
    worldUp.normalize(); // Z 
    
  
    Ogre::Matrix3 m3x3;

    m3x3[0][0] = xVec.x;
 	m3x3[0][1] = front.x;
 	m3x3[0][2] = worldUp.x;
 	m3x3[1][0] = xVec.y;
 	m3x3[1][1] = front.y;
 	m3x3[1][2] = worldUp.y;
 	m3x3[2][0] = xVec.z;
 	m3x3[2][1] = front.z;
 	m3x3[2][2] = worldUp.z; 
 
    Ogre::Quaternion q(m3x3);
    Quaternion orientation(q.x, q.y,q.z, q.w);
    
    Vector3df rotations;
    orientation.toEuler(rotations);
    
    rotations*=RADTODEG;
    return rotations;

}

void EC_Terrain::GetTriangleNormals(float x, float y, Vector3df &n1, Vector3df &n2, Vector3df &n3, float &u, float &v) const
{
    x = max(0.f, min((float)VerticesWidth()-1.f, x));
    y = max(0.f, min((float)VerticesHeight()-1.f, y));

    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    xFloor = clamp(xFloor, 0, PatchWidth() * cPatchSize - 1);
    xCeil = clamp(xCeil, 0, PatchWidth() * cPatchSize - 1);
    yFloor = clamp(yFloor, 0, PatchHeight() * cPatchSize - 1);
    yCeil = clamp(yCeil, 0, PatchHeight() * cPatchSize - 1);

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

void EC_Terrain::GetTriangleVertices(float x, float y, Vector3df &v1, Vector3df &v2, Vector3df &v3, float &u, float &v) const
{
    x = max(0.f, min((float)VerticesWidth()-1.f, x));
    y = max(0.f, min((float)VerticesHeight()-1.f, y));

    int xFloor = (int)floor(x);
    int xCeil = (int)ceil(x);
    int yFloor = (int)floor(y);
    int yCeil = (int)ceil(y);

    xFloor = clamp(xFloor, 0, PatchWidth() * cPatchSize - 1);
    xCeil = clamp(xCeil, 0, PatchWidth() * cPatchSize - 1);
    yFloor = clamp(yFloor, 0, PatchHeight() * cPatchSize - 1);
    yCeil = clamp(yCeil, 0, PatchHeight() * cPatchSize - 1);

    float xFrac = fmod(x, 1.f);
    float yFrac = fmod(y, 1.f);

    v2 = Vector3df((float)xFloor, (float)yCeil, GetPoint(xFloor, yCeil));
    v3 = Vector3df((float)xCeil, (float)yFloor, GetPoint(xCeil, yFloor));

    if (xFrac + yFrac >= 1.f)
    {
        //if xFrac >= yFrac
        v1 = Vector3df((float)xCeil, (float)yCeil, GetPoint(xCeil, yCeil));
        xFrac = 1.f - xFrac;
        yFrac = 1.f - yFrac;
    }
    else
    {
        v1 = Vector3df((float)xFloor, (float)yFloor, GetPoint(xFloor, yFloor));
        swap(v2, v3);
    }
    u = xFrac;
    v = yFrac;
}

Vector3df EC_Terrain::GetPlaneNormal(float x, float y) const
{
    Vector3df h1, h2, h3;
    float u, v;
    GetTriangleVertices(x, y, h1, h2, h3, u, v);

    // h1 to h3 are the three terrain height points in local coordinate space.
    Vector3df normal = (h3-h2).crossProduct(h3-h1);
    Ogre::Vector4 oNormal = Ogre::Vector4(normal.x, normal.y, normal.z, 0.f);

    Ogre::Matrix4 worldTM = GetWorldTransform(rootNode);
    oNormal = worldTM * oNormal;
    normal = Vector3df(oNormal.x, oNormal.y, oNormal.z);
    normal.normalize();

    return normal;
}

Vector3df EC_Terrain::GetInterpolatedNormal(float x, float y) const
{
    Vector3df n1, n2, n3;
    float u, v;
    GetTriangleNormals(x, y, n1, n2, n3, u, v);

    // h1 to h3 are the three terrain height points in local coordinate space.
    Vector3df normal = (1.f - u - v) * n1 + u * n2 + v * n3;
    Ogre::Vector4 oNormal = Ogre::Vector4(normal.x, normal.y, normal.z, 0.f);

    Ogre::Matrix4 worldTM = GetWorldTransform(rootNode);
    oNormal = worldTM * oNormal;
    normal = Vector3df(oNormal.x, oNormal.y, oNormal.z);
    normal.normalize();

    return normal;
}

Vector3df EC_Terrain::CalculateNormal(int x, int y, int xinside, int yinside) const
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

bool EC_Terrain::SaveToFile(QString filename)
{
    if (patchWidth * patchHeight != (int)patches.size())
    {
        LogError("The EC_Terrain is in inconsistent state. Cannot save.");
        return false;
    }

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

    for(int i = 0; i < xPatches*yPatches; ++i)
    {
        if (patches[i].heightData.size() < cPatchSize*cPatchSize)
            patches[i].heightData.resize(cPatchSize*cPatchSize);

        fwrite(&patches[i].heightData[0], sizeof(float), cPatchSize*cPatchSize, handle); ///< \todo Check read error.
    }
    fclose(handle);

    return true;
}

bool LoadFileToVector(const char *filename, std::vector<u8> &dst)
{
    FILE *handle = fopen(filename, "rb");
    if (!handle)
    {
        LogError("Could not open file " + std::string(filename) + ".");
        return false;
    }

    fseek(handle, 0, SEEK_END);
    long numBytes = ftell(handle);
    if (numBytes == 0)
    {
        fclose(handle);
        return false;
    }

    fseek(handle, 0, SEEK_SET);
    dst.resize(numBytes);
    size_t numRead = fread(&dst[0], sizeof(u8), numBytes, handle);
    fclose(handle);

    return (long)numRead == numBytes;
}

u32 ReadU32(const char *dataPtr, size_t numBytes, int &offset)
{
    if (offset + 4 > numBytes)
        throw Exception("Not enough bytes to deserialize!");
    u32 data = *(u32*)(dataPtr + offset); ///\note Requires unaligned load support from the CPU and assumes data storage endianness to be the same for loader and saver.
    offset += 4;
    return data;
}

bool EC_Terrain::LoadFromFile(QString filename)
{
    filename = filename.trimmed();

    std::vector<u8> file;
    LoadFileToVector(filename.toStdString().c_str(), file);

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

bool EC_Terrain::LoadFromImageFile(QString filename, float offset, float scale)
{
    Ogre::Image image;
    try
    {
        std::vector<u8> imageFile;
        LoadFileToVector(filename.toStdString().c_str(), imageFile);
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&imageFile[0], imageFile.size(), false));
        image.load(stream);
    } catch(...)
    {
        LogError("Execption catched when trying load terrain from image file" + filename.toStdString() + ".");
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

    heightMap.Set(AssetReference(""/*,""*/), AttributeChange::Disconnected);

    xPatches.Changed(AttributeChange::LocalOnly);
    yPatches.Changed(AttributeChange::LocalOnly);
    heightMap.Changed(AttributeChange::LocalOnly);

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
    } catch(...)
    {
        ///\todo Log out warning.
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

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
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
    Scene::Entity *parentEntity = GetParentEntity();
    if (!parentEntity)
        return;

    Scene::SceneManager *scene = parentEntity->GetScene();
    if (!scene)
        return;

    Scene::Entity *entity = scene->GetEntityByNameRaw(entityName);
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
        LogError("Could not get mesh " + ogreMeshResourceName.toStdString() + ".");
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

    int xVertices = (int)ceil(maxExtents.x - minExtents.x);
    int yVertices = (int)ceil(maxExtents.y - minExtents.y);
    xVertices = ((xVertices + cPatchSize-1) / cPatchSize) * cPatchSize;
    yVertices = ((yVertices + cPatchSize-1) / cPatchSize) * cPatchSize;

    xPatches.Set(xVertices/cPatchSize, AttributeChange::Disconnected);
    yPatches.Set(yVertices/cPatchSize, AttributeChange::Disconnected);
    ResizeTerrain(xVertices/cPatchSize, yVertices/cPatchSize);

    MakeTerrainFlat(1e9f);

    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = -std::numeric_limits<float>::max();

    const float raycastHeight = maxExtents.z + 100.f;

    for(int y = 0; y < yVertices; ++y)
        for(int x = 0; x < xVertices; ++x)
        {
            Ogre::Ray r(Ogre::Vector3(minExtents.x + x, minExtents.y + y, raycastHeight), Ogre::Vector3(0,0,-1.f));
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

void EC_Terrain::SetTerrainMaterialTexture(int index, const char *textureName)
{
    if (index < 0 || index > 4)
        return;

//    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
//    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(textureName).get());

    Ogre::MaterialPtr terrainMaterial = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
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
            sub->setMaterialName(currentMaterial.toStdString().c_str());
    }
}

void EC_Terrain::UpdateRootNodeTransform()
{
    if (!rootNode)
        return;

    const Transform &tm = nodeTransformation.Get();

    Ogre::Matrix3 rot_new;
    rot_new.FromEulerAnglesXYZ(Ogre::Degree(tm.rotation.x), Ogre::Degree(tm.rotation.y), Ogre::Degree(tm.rotation.z));
    Ogre::Quaternion q_new(rot_new);

    rootNode->setOrientation(Ogre::Quaternion(rot_new));
    rootNode->setPosition(tm.position.x, tm.position.y, tm.position.z);
    rootNode->setScale(tm.scale.x, tm.scale.y, tm.scale.z);
}

void EC_Terrain::AttachTerrainRootNode()
{
    if (!rootNode)
        return;

    OgreRenderer::RendererPtr renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    if (!sceneMgr)
        return;

    // Detach the terrain root node from any previous EC_Placeable scenenode.
    if (rootNode->getParentSceneNode())
        rootNode->getParentSceneNode()->removeChild(rootNode);

    // If this entity has an EC_Placeable, make sure it is the parent of this terrain component.
    boost::shared_ptr<EC_Placeable> pos = GetParentEntity()->GetComponent<EC_Placeable>();
    if (pos.get())
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

/// Creates Ogre geometry data for the single given patch, or updates the geometry for an existing
/// patch if the associated Ogre resources already exist.
void EC_Terrain::GenerateTerrainGeometryForOnePatch(int patchX, int patchY)
{
    PROFILE(EC_Terrain_GenerateTerrainGeometryForOnePatch);

    EC_Terrain::Patch &patch = GetPatch(patchX, patchY);

    Renderer *renderer = framework_->GetService<Renderer>();
    if (!renderer)
        return;
    if (!ViewEnabled())
        return;

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

            // The UV set 0 contains the diffuse texture UV map. Do a planar mapping with the given specified UV scale.
// Ogre:                manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.z + pos.z) * vScale);
            manual->textureCoord((patchOrigin.x + pos.x) * uScale, (patchOrigin.y + pos.y) * vScale);

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

    OgreRenderer::RendererPtr renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();
    if (!sceneMgr)
        return;

    rootNode = sceneMgr->createSceneNode();

    // Add the newly created node to scene or to a parent EC_Placeable.
    AttachTerrainRootNode();

    UpdateRootNodeTransform();
}

void EC_Terrain::CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY)
{
    OgreRenderer::RendererPtr renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer).lock();
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

float EC_Terrain::GetTerrainMinHeight() const
{
    float minHeight = std::numeric_limits<float>::max();

    for(int i = 0; i < patches.size(); ++i)
        for(int j = 0; j < patches[i].heightData.size(); ++j)
            minHeight = min(minHeight, patches[i].heightData[j]);

    return minHeight;
}

float EC_Terrain::GetTerrainMaxHeight() const
{
    float maxHeight = -std::numeric_limits<float>::max();

    for(int i = 0; i < patches.size(); ++i)
        for(int j = 0; j < patches[i].heightData.size(); ++j)
            maxHeight = max(maxHeight, patches[i].heightData[j]);

    return maxHeight;
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
    
    // All the new geometry we created will be visible for Ogre by default. If the EC_Placeable's visible attribute is false,
    // we need to hide all newly created geometry.
    AttachTerrainRootNode();

    ///\todo If this terrain only exists for physics heightfield purposes, don't create GPU resources for it at all.

    emit TerrainRegenerated();
}

}
