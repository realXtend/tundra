// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_OgreMeshAsset_h
#define incl_OgreRenderingModule_OgreMeshAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"

#include <OgreMesh.h>
#include <OgreResourceBackgroundQueue.h>

class OgreMeshAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{

Q_OBJECT

public:
    /// Constructor.
    OgreMeshAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    /// Deconstructor.
    ~OgreMeshAsset();

    /// Load mesh from memory. IAsset override.
    virtual AssetLoadState DeserializeFromData(const u8 *data_, size_t numBytes);

    /// Load mesh into memory. IAsset override.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

    /// Unload mesh from ogre. IAsset override.
    virtual void DoUnload();

    /// Handle load errors detected by AssetAPI. IAsset override.
    virtual void HandleLoadError(const QString &loadError);

    /// Returns an empty list - meshes do not refer to other assets. IAsset override.
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
};

typedef boost::shared_ptr<OgreMeshAsset> OgreMeshAssetPtr;

#endif
