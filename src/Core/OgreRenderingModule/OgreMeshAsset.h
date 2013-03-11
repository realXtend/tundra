// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "Math/MathNamespace.h"
#include "IAsset.h"
#include "OgreModuleApi.h"

#include <OgreMesh.h>
#include <OgreResourceBackgroundQueue.h>
#include "Math/float2.h"
#include "Geometry/KdTree.h"
#include "Geometry/Triangle.h"
#include "IRenderer.h"

class OpenAssetImport;

/// Represents an Ogre .mesh loaded to the GPU.
class OGRE_MODULE_API OgreMeshAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{
    Q_OBJECT

public:
    OgreMeshAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
        IAsset(owner, type_, name_), loadTicket_(0)
    {
    }

    ~OgreMeshAsset();

    virtual bool LoadFromFile(QString filename);

    /// Load mesh from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, bool allowAsynchronous);

    /// Load mesh into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

    /// Unload mesh from ogre
    virtual void DoUnload();

    /// Returns an empty list - meshes do not refer to other assets.
    virtual std::vector<AssetReference> FindReferences() const { return std::vector<AssetReference>(); }

    void SetDefaultMaterial();

    bool IsLoaded() const;

    /// This points to the loaded mesh asset, if it is present.
    Ogre::MeshPtr ogreMesh;

    /// Ticket for ogres threaded loading operation.
    Ogre::BackgroundProcessTicket loadTicket_;

    /// Specifies the unique mesh name Ogre uses in its asset pool for this mesh.
    //QString ogreAssetName;

    //std::vector<QString> originalMaterials;

public slots:
    RayQueryResult Raycast(const Ray &ray);

    /// Returns the given triangle of the mesh data.
    Triangle Tri(int submeshIndex, int triangleIndex);

    int NumSubmeshes();

    int NumTris(int submeshIndex);

#ifdef ASSIMP_ENABLED
private slots:
    void OnAssimpConversionDone(bool);
#endif

private:
    /// Precomputes a kD-tree for the triangle data of this mesh.
    void CreateKdTree();

    /// Process mesh data after loading to create tangents and such.
    bool GenerateMeshData();

    void ConvertAssimpDataToOgreMesh(const u8 *data_, size_t numBytes);

    bool IsAssimpFileType();

    /// Stores a CPU-side version of the mesh geometry data (positions), for raycasting purposes.
    KdTree<Triangle> meshData;

    std::vector<float3> normals; ///< Triangle normals. One per triangle (not per-vertex normals).
    std::vector<float2> uvs; 
    std::vector<int> subMeshTriangleCounts;

#ifdef ASSIMP_ENABLED
    OpenAssetImport *importer;
#endif

};
