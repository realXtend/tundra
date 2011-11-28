// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include "OgreModuleApi.h"

#include <OgreMesh.h>
#include <OgreResourceBackgroundQueue.h>

/// Represents an Ogre .mesh loaded to the GPU.
class OGRE_MODULE_API OgreMeshAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{
    Q_OBJECT

public:
    OgreMeshAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    ~OgreMeshAsset();

    /// Load mesh from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, const bool allowAsynchronous);

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

private:
    /// Process mesh data after loading to create tangents and such.
    void GenerateMeshdata();

};

typedef boost::shared_ptr<OgreMeshAsset> OgreMeshAssetPtr;

