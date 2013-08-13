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

/// Represents an Ogre mesh loaded to the GPU.
class OGRE_MODULE_API OgreMeshAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{
    Q_OBJECT

public:
    OgreMeshAsset(AssetAPI *owner, const QString &type_, const QString &name_);
    ~OgreMeshAsset();

    /// Load mesh from file. IAsset override.
    virtual bool LoadFromFile(QString filename);

    /// Load mesh from memory. IAsset override.
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, bool allowAsynchronous);

    /// Load mesh into memory. IAsset override.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

    /// Loaded Ogre mesh asset, null if not loaded.
    Ogre::MeshPtr ogreMesh;

public slots:
    /// IAsset override.
    virtual bool IsLoaded() const;

    /// Returns Ogres internal asset name.
    QString OgreMeshName() const;

    /// Executes raycast to the CPU-side cached geometry.
    RayQueryResult Raycast(const Ray &ray);

    /// Returns the given triangle of the mesh data.
    Triangle Tri(int submeshIndex, int triangleIndex);

    /// Returns submesh count.
    size_t NumSubmeshes();

    /// Returns triangle count for submesh.
    int NumTris(int submeshIndex);

    /// Is this mesh a Assimp supported file type.
    bool IsAssimpFileType() const;

signals:
    void ExternalConversionRequested(OgreMeshAsset*, const u8*, size_t);

#ifdef ASSIMP_ENABLED
public slots:
    void OnAssimpConversionDone(bool);
#endif

private:
    /// Unload mesh from Ogre. IAsset override.
    virtual void DoUnload();

    /// Precomputes a kD-tree for the triangle data of this mesh.
    void CreateKdTree();

    /// Process mesh data after loading to create tangents and such.
    bool GenerateMeshData();

    /// Sets default material.
    void SetDefaultMaterial();

    /// Ticket for ogres threaded loading operation.
    Ogre::BackgroundProcessTicket loadTicket_;

    /// Stores a CPU-side version of the mesh geometry data (positions), for raycasting purposes.
    KdTree<Triangle> meshData;

    /// Triangle normals. One per triangle (not per-vertex normals).
    std::vector<float3> normals;

    /// UVs.
    std::vector<float2> uvs; 

    /// Triangle counts per submesh.
    std::vector<int> subMeshTriangleCounts;
};
