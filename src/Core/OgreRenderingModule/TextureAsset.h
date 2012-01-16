// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <OgreTexture.h>

#include <boost/shared_ptr.hpp>
#include "IAsset.h"
#include "AssetAPI.h"
#include <QImage>
#include "OgreModuleApi.h"
#include <OgreResourceBackgroundQueue.h>

/// Represents a texture on the GPU.
class OGRE_MODULE_API TextureAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{
    Q_OBJECT

public:
    TextureAsset(AssetAPI *owner, const QString &type, const QString &name);
    ~TextureAsset();

    /// Load texture from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, const bool allowAsynchronous);

    /// Load texture into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

    /// Unload texture from ogre
    virtual void DoUnload();

    /// Convert texture to QImage
    QImage ToQImage(size_t faceIndex = 0, size_t mipmapLevel = 0) const;

    bool IsLoaded() const;

    /// Sets the contents of this texture asset from raw pixel data.
    /** @param newWidth The desired pixel width for this texture.
        @param newHeight The desired pixel height for this texture. If newWidth or newHeight do not match with the current texture size on the GPU side,
            the texture is recreated to the desired size.
        @param data A pointer to the raw pixel data of the image to set to this Texture. This data must be in interleaved A8R8G8B8 format. If you only want to resize the 
            texture, and not fill it with any data at this time, you may pass in a null pointer here.
        @param numBytes The size of array pointed by data. This is expected to be newWidth*newHeight*4.
        @param ogreFormat The texture format to use for this texture. Currently pass in only Ogre::PF_A8R8G8B8.
        @param regenerateMipmaps If true, the new texture will be created to use mipmapping, and a full mipmap chain will be automatically created for this texture (better image quality,
            but makes this function call considerably slower).
            If false, only the first mip level is generated (worse image quality if the texture is viewed from afar, but this function call proceeds faster).
        @param dynamic Hints the GPU whether this texture surface is updated often and should be made a dynamic GPU resource.
        @param renderTarget If true, this texture will be used as a GPU render target. 
        @todo Add option to submit individual surfaces.
        @todo Add support for different pixel formats */
    void SetContents(size_t newWidth, size_t newHeight, const u8 *data, size_t numBytes, Ogre::PixelFormat ogreFormat, bool regenerateMipmaps, bool dynamic, bool renderTarget/* = false*/);

    /// Sets this texture to the given size and fills it with the given color value.
    void SetContentsFillSolidColor(int newWidth, int newHeight, u32 color, Ogre::PixelFormat ogreFormat, bool regenerateMipmaps, bool dynamic);

    /// Renderes the given text to a texture.
    /** This function uses Qt QImage and QPainter objects to draw the specified text to an image, and then calls SetContents() to upload that image to this TextureAsset.
        @see SetContents(). */
    void SetContentsDrawText(int newWidth, int newHeight, QString text, const QColor &textColor, const QFont &font, const QBrush &backgroundBrush, const QPen &borderPen, 
        int flags/* = Qt::AlignCenter | Qt::TextWordWrap*/, bool generateMipmaps/* = false*/, bool dynamic/* = false*/, float xRadius/* = 20.f*/, float yRadius/*= 20.f*/);

    //void RegenerateAllMipLevels();

    /// This points to the loaded texture asset, if it is present.
    Ogre::TexturePtr ogreTexture;

    /// Specifies the unique texture name Ogre uses in its asset pool for this texture.
    QString ogreAssetName;

    /// Ticket for ogres threaded loading operation.
    Ogre::BackgroundProcessTicket loadTicket_;
    
    /// Convert texture to QImage, static version.
    static QImage ToQImage(Ogre::Texture* tex, size_t faceIndex = 0, size_t mipmapLevel = 0);
};

typedef boost::shared_ptr<TextureAsset> TextureAssetPtr;

