// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_OgreMeshAsset_h
#define incl_OgreRenderingModule_OgreMeshAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include <OgreMesh.h>

class OgreMeshAsset : public IAsset
{

Q_OBJECT

public:
    OgreMeshAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    ~OgreMeshAsset();

    /// Load mesh from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    /// Loade mesh into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters);

    /// Unload mesh from ogre
    virtual void DoUnload();

    /// Handle load errors detected by AssetAPI
    virtual void HandleLoadError(const QString &loadError);

    /// Returns an empty list - meshes do not refer to other assets.
    virtual std::vector<AssetReference> FindReferences() const { return std::vector<AssetReference>(); }

    void SetDefaultMaterial();

    bool IsLoaded() const;

    /// This points to the loaded mesh asset, if it is present.
    Ogre::MeshPtr ogreMesh;

    /// Specifies the unique mesh name Ogre uses in its asset pool for this mesh.
    //QString ogreAssetName;

    //std::vector<QString> originalMaterials;
};

typedef boost::shared_ptr<OgreMeshAsset> OgreMeshAssetPtr;

#endif
