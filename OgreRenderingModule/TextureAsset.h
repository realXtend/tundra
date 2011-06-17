// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_TextureAsset_h
#define incl_OgreRenderingModule_TextureAsset_h

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include "AssetAPI.h"

#include <OgreTexture.h>
#include <OgreResourceBackgroundQueue.h>

class TextureAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{

Q_OBJECT

public:
    /// Constructor.
    TextureAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    /// Deconstructor.
    ~TextureAsset();

    /// Load texture from memory. IAsset override.
    virtual AssetLoadState DeserializeFromData(const u8 *data_, size_t numBytes);

    /// Load texture into memory. IAsset override.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

    /// Handle load errors detected by AssetAPI. IAsset override.
    virtual void HandleLoadError(const QString &loadError);

    /// Unload texture from ogre
    virtual void DoUnload();   

    bool IsLoaded() const;

    //void RegenerateAllMipLevels();

    /// This points to the loaded texture asset, if it is present.
    Ogre::TexturePtr ogreTexture;

    /// Specifies the unique texture name Ogre uses in its asset pool for this texture.
    QString ogreAssetName;

    Ogre::BackgroundProcessTicket loadTicket_;
};

typedef boost::shared_ptr<TextureAsset> TextureAssetPtr;

#endif
