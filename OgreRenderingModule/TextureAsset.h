// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_TextureAsset_h
#define incl_OgreRenderingModule_TextureAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include <OgreTexture.h>

class TextureAsset : public IAsset
{
    Q_OBJECT;
public:
    TextureAsset(AssetAPI *owner, const QString &type_, const QString &name_)
    :IAsset(owner, type_, name_)
    {
    }

    ~TextureAsset();

    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters);

    virtual void DoUnload();

    /// Returns an empty list - textures do not refer to other assets.
    virtual std::vector<AssetReference> FindReferences() const { return std::vector<AssetReference>(); }

//    void RegenerateAllMipLevels();

    /// This points to the loaded texture asset, if it is present.
    Ogre::TexturePtr ogreTexture;

    /// Specifies the unique texture name Ogre uses in its asset pool for this texture.
    QString ogreAssetName;
};

typedef boost::shared_ptr<TextureAsset> TextureAssetPtr;

#endif
