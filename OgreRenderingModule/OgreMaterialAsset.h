// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreMaterialAsset_h
#define incl_OgreRenderer_OgreMaterialAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include <OgreMaterial.h>

class OgreMaterialAsset : public IAsset
{

Q_OBJECT

public:
    OgreMaterialAsset(AssetAPI *owner, const QString &type_, const QString &name_) : IAsset(owner, type_, name_) {}
    ~OgreMaterialAsset();

    /// Load material from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    /// Load material into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters);

    /// Unload material from ogre
    virtual void DoUnload();

    /// Handle load errors detected by AssetAPI
    virtual void HandleLoadError(const QString &loadError);

    /// Return references of this material, in most cases textures
    virtual std::vector<AssetReference> FindReferences() const;

    bool IsLoaded() const;

    /// Material ptr to the asset in ogre
    Ogre::MaterialPtr ogreMaterial;

    /// Specifies the unique texture name Ogre uses in its asset pool for this material.
    QString ogreAssetName;

    /// references to other resources this resource depends on
    std::vector<AssetReference> references_;
};

#endif


