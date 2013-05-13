// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleApi.h"
#include "IAsset.h"
#include "AssetAPI.h"

#include <QImage>

#include <OgreTexture.h>
#include <OgreResourceBackgroundQueue.h>

/// Represents a texture on the GPU.
class OGRE_MODULE_API TextureAsset : public IAsset, Ogre::ResourceBackgroundQueue::Listener
{
    Q_OBJECT

public:
    TextureAsset(AssetAPI *owner, const QString &type, const QString &name);
    ~TextureAsset();

    virtual bool LoadFromFile(QString filename);

    /// Load texture from memory
    virtual bool DeserializeFromData(const u8 *data_, size_t numBytes, bool allowAsynchronous);

    /// Load texture into memory
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const;

    /// Ogre threaded load listener. Ogre::ResourceBackgroundQueue::Listener override.
    virtual void operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result);

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

    /// Perform post-processing after loading (DXT compression and size reduction) according to command line options
    void PostProcessTexture();
    
    /// Compress texture to suitable DXT format. Also, if applicable, reduce texture size at the same time.
    void CompressTexture();

    /// Reduce texture size only according to command line options
    void ReduceTextureSize();
    
    /// This points to the loaded texture asset, if it is present.
    Ogre::TexturePtr ogreTexture;

    /// Specifies the unique texture name Ogre uses in its asset pool for this texture.
    QString ogreAssetName;

    /// Ticket for ogres threaded loading operation.
    Ogre::BackgroundProcessTicket loadTicket_;
    
    /// Convert texture to QImage, static version.
    static QImage ToQImage(Ogre::Texture* tex, size_t faceIndex = 0, size_t mipmapLevel = 0);

    /** This function is used internally to convert our name into Ogre form.
        Meaning we swap file suffixes that Ogre does not support to ones it supports
        and handle conversion before passing to Ogre in DeserializeFrom. */
    QString NameInternal() const;

    /// Same as NameInternal but static and takes the textureRef as a parameter.
    static QString NameInternal(const QString &textureRef);

    /// Decompresses any CRN input data to DDS.
    /** @param crnData Ptr to compressed crn data.
     ** @param crnNumBytes Size of crn data in bytes.
     ** @param outDdsNumByte Size of decompressed dds data in bytes is assigned to this parameter.
     ** @return void* to the uncompressed DDS data. It is the callers responsibility to free the 
     ** data with crn_free_block when done with it. */
    bool DecompressCRNtoDDS(const u8 *crnData, size_t crnNumBytes, std::vector<u8> &ddsData);

public slots:
    /// Convert texture to QImage
    QImage ToQImage(size_t faceIndex = 0, size_t mipmapLevel = 0) const;

    /// Get height of the texture. Return 0 if not loaded.
    size_t Height() const;

    /// Get width of the texture. Return 0 if not loaded.
    size_t Width() const;

    /// Texture extension.
    QString NameSuffix() const;

private:
    /// Unload texture from ogre
    virtual void DoUnload();
};
