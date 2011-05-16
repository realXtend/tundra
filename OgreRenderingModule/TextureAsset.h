// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_TextureAsset_h
#define incl_OgreRenderingModule_TextureAsset_h

#include <OgreTexture.h>

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include "AssetAPI.h"
#include <QImage>
#include "OgreModuleApi.h"

class OGRE_MODULE_API TextureAsset : public IAsset
{
    Q_OBJECT;
public:
    TextureAsset(AssetAPI *owner, const QString &type, const QString &name);
    ~TextureAsset();

    /// Load texture from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes);

    /// Load texture into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Handle load errors detected by AssetAPI
    virtual void HandleLoadError(const QString &loadError);

    /// Unload texture from ogre
    virtual void DoUnload();   

    QImage ToQImage(size_t faceIndex = 0, size_t mipmapLevel = 0) const;

    bool IsLoaded() const;

    ///\todo Implement regenerateMipmaps option.
    ///\todo Add individual surface set option.
    /// @param data The new contents of the texture. If you only want to resize the texture, and not fill it
    ///     with any data at this time, you may pass in a null pointer here.
    void SetContents(int newWidth, int newHeight, const u8 *data, size_t numBytes, Ogre::PixelFormat ogreFormat, bool regenerateMipmaps);

    /// Sets this texture to the given size and fills it with the given color value.
    void SetContentsFillSolidColor(int newWidth, int newHeight, u32 color, Ogre::PixelFormat ogreFormat, bool regenerateMipmaps);

    /// Sets given text to a texture.
    void SetTextContent(int newWidth, int newHeight, const QString& text, const QColor& textColor, const QFont& font, const QBrush& backgroundBrush, const QPen& borderPen, int flags = Qt::AlignCenter | Qt::TextWordWrap);

    //void RegenerateAllMipLevels();

    /// This points to the loaded texture asset, if it is present.
    Ogre::TexturePtr ogreTexture;

    /// Specifies the unique texture name Ogre uses in its asset pool for this texture.
    QString ogreAssetName;
};

typedef boost::shared_ptr<TextureAsset> TextureAssetPtr;

#endif
