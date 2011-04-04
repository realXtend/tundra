// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_TextureAsset_h
#define incl_OgreRenderingModule_TextureAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include "AssetAPI.h"
#include <OgreTexture.h>

class TextureAsset : public IAsset
{
    Q_OBJECT;
public:
    TextureAsset(AssetAPI *owner, const QString &type_, const QString &name_) : IAsset(owner, type_, name_) {}
    ~TextureAsset();

    /// Load texture from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    /// Load texture into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters);

    /// Handle load errors detected by AssetAPI
    virtual void HandleLoadError(const QString &loadError);

    /// Unload texture from ogre
    virtual void DoUnload();   

    bool IsLoaded() const;

    //void RegenerateAllMipLevels();

    /// This points to the loaded texture asset, if it is present.
    Ogre::TexturePtr ogreTexture;

    /// Specifies the unique texture name Ogre uses in its asset pool for this texture.
    QString ogreAssetName;
};

typedef boost::shared_ptr<TextureAsset> TextureAssetPtr;

#endif
