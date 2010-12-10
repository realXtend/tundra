// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreMaterialAsset_h
#define incl_OgreRenderer_OgreMaterialAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include <OgreMaterial.h>

class OgreMaterialAsset : public IAsset
{
    Q_OBJECT;
public:
    OgreMaterialAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    ~OgreMaterialAsset();

    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters);

    virtual void Unload();

    virtual std::vector<AssetReference> FindReferences() const;

    Ogre::MaterialPtr ogreMaterial;

    /// Specifies the unique texture name Ogre uses in its asset pool for this material.
    QString ogreAssetName;

    //! references to other resources this resource depends on
    std::vector<AssetReference> references_;
};

#endif


