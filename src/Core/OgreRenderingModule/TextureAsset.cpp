// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TextureAsset.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"

#include "Profiler.h"
#include "AssetCache.h"
#include "LoggingFunctions.h"

#include <QPixmap>
#include <QRect>
#include <QFontMetrics>
#include <QPainter>
#include <QFileInfo>

#include <Ogre.h>

#include <crn_decomp.h>
#include <dds_defs.h>

#ifdef WIN32
#include <squish.h>
#endif

#if defined(DIRECTX_ENABLED) && defined(WIN32)
#ifdef SAFE_DELETE
#undef SAFE_DELETE
#endif
#ifdef SAFE_DELETE_ARRAY
#undef SAFE_DELETE_ARRAY
#endif
#include <d3d9.h>
#include <OgreD3D9RenderSystem.h>
#include <OgreD3D9HardwarePixelBuffer.h>
#endif

#include "MemoryLeakCheck.h"

const float BUDGET_THRESHOLD = 0.80f; // The point at which we start reducing texture size
const float BUDGET_STEP = 0.05f; // The step at which texture maximum size limit is halved

TextureAsset::TextureAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
    IAsset(owner, type_, name_), loadTicket_(0)
{
    ogreAssetName = AssetAPI::SanitateAssetRef(NameInternal());
}

TextureAsset::~TextureAsset()
{
    Unload();
}

bool TextureAsset::LoadFromFile(QString filename)
{
    bool allowAsynchronous = AllowAsyncLoading();

    QString cacheDiskSource;
    if (allowAsynchronous)
    {
        cacheDiskSource = assetAPI->Cache()->FindInCache(Name());
        if (cacheDiskSource.isEmpty())
            allowAsynchronous = false;
    }
    
    if (allowAsynchronous)
        return DeserializeFromData(0, 0, true);
    else
        return IAsset::LoadFromFile(filename);
}

QString TextureAsset::NameInternal() const
{
    return NameInternal(name);
}

QString TextureAsset::NameInternal(const QString &textureRef)
{
    // .crn -> .dds
    if (textureRef.endsWith(".crn", Qt::CaseInsensitive))
        return textureRef.left(textureRef.lastIndexOf(".")+1) + "dds";
    return textureRef;
}

QString TextureAsset::NameSuffix() const
{
    QString checkName = name;
    if (checkName.contains("/"))
        checkName = checkName.mid(checkName.lastIndexOf("/")+1);
    return QFileInfo(checkName).suffix().toLower();
}

bool TextureAsset::DecompressCRNtoDDS(const u8 *crnData, size_t crnNumBytes, std::vector<u8> &ddsData)
{
    PROFILE(TextureAsset_DeserializeFromData_CRN_Uncompress);
    ddsData.clear();
    
    // Texture data
    crnd::crn_texture_info textureInfo;
    if (!crnd::crnd_get_texture_info((void*)crnData, (crnd::uint32)crnNumBytes, &textureInfo))
    {
        LogError("CRN texture info parsing failed, invalid input data.");
        return false;
    }
    // Begin unpack
    crnd::crnd_unpack_context crnContext = crnd::crnd_unpack_begin((void*)crnData, (crnd::uint32)crnNumBytes);
    if (!crnContext)
    {
        LogError("CRN texture data unpacking failed, invalid input data.");
        return false;
    }
    
    // DDS header
    crnlib::DDSURFACEDESC2 header;
    memset(&header, 0, sizeof(header));
    header.dwSize = sizeof(header);
    // - Size and flags
    header.dwFlags = crnlib::DDSD_CAPS | crnlib::DDSD_HEIGHT | crnlib::DDSD_WIDTH | crnlib::DDSD_PIXELFORMAT | ((textureInfo.m_levels > 1) ? crnlib::DDSD_MIPMAPCOUNT : 0);
    header.ddsCaps.dwCaps = crnlib::DDSCAPS_TEXTURE;
    header.dwWidth = textureInfo.m_width;
    header.dwHeight = textureInfo.m_height;
    // - Pixelformat
    header.ddpfPixelFormat.dwSize = sizeof(crnlib::DDPIXELFORMAT);
    header.ddpfPixelFormat.dwFlags = crnlib::DDPF_FOURCC;
    crn_format fundamentalFormat = crnd::crnd_get_fundamental_dxt_format(textureInfo.m_format);
    header.ddpfPixelFormat.dwFourCC = crnd::crnd_crn_format_to_fourcc(fundamentalFormat);
    if (fundamentalFormat != textureInfo.m_format)
        header.ddpfPixelFormat.dwRGBBitCount = crnd::crnd_crn_format_to_fourcc(textureInfo.m_format);
    // - Mipmaps
    header.dwMipMapCount = (textureInfo.m_levels > 1) ? textureInfo.m_levels : 0;
    if (textureInfo.m_levels > 1)
        header.ddsCaps.dwCaps |= (crnlib::DDSCAPS_COMPLEX | crnlib::DDSCAPS_MIPMAP);
    // - Cubemap with 6 faces
    if (textureInfo.m_faces == 6)
    {
        header.ddsCaps.dwCaps2 = crnlib::DDSCAPS2_CUBEMAP | 
            crnlib::DDSCAPS2_CUBEMAP_POSITIVEX | crnlib::DDSCAPS2_CUBEMAP_NEGATIVEX | crnlib::DDSCAPS2_CUBEMAP_POSITIVEY | 
            crnlib::DDSCAPS2_CUBEMAP_NEGATIVEY | crnlib::DDSCAPS2_CUBEMAP_POSITIVEZ | crnlib::DDSCAPS2_CUBEMAP_NEGATIVEZ;
    }
    
    // Set pitch/linear size field (some DDS readers require this field to be non-zero).
    int bits_per_pixel = crnd::crnd_get_crn_format_bits_per_texel(textureInfo.m_format);
    header.lPitch = (((header.dwWidth + 3) & ~3) * ((header.dwHeight + 3) & ~3) * bits_per_pixel) >> 3;
    header.dwFlags |= crnlib::DDSD_LINEARSIZE;
          
    // Prepare output data
    uint totalSize = sizeof(crnlib::cDDSFileSignature) + header.dwSize;
    uint writePos = 0;
    ddsData.resize(totalSize);

    // Write signature. Note: Not endian safe.
    memcpy(&ddsData[0] + writePos, &crnlib::cDDSFileSignature, sizeof(crnlib::cDDSFileSignature));
    writePos += sizeof(crnlib::cDDSFileSignature);

    // Write header
    memcpy(&ddsData[0] + writePos, &header, header.dwSize);
    writePos += header.dwSize;
    
    // Now transcode all face and mipmap levels into memory, one mip level at a time.    
    for (crn_uint32 iLevel = 0; iLevel < textureInfo.m_levels; iLevel++)
    {
        // Compute the face's width, height, number of DXT blocks per row/col, etc.
        const crn_uint32 width = std::max(1U, textureInfo.m_width >> iLevel);
        const crn_uint32 height = std::max(1U, textureInfo.m_height >> iLevel);
        const crn_uint32 blocksX = std::max(1U, (width + 3) >> 2);
        const crn_uint32 blocksY = std::max(1U, (height + 3) >> 2);
        const crn_uint32 rowPitch = blocksX * crnd::crnd_get_bytes_per_dxt_block(textureInfo.m_format);
        const crn_uint32 faceSize = rowPitch * blocksY;

        totalSize += faceSize;
        if (ddsData.size() < totalSize)
            ddsData.resize(totalSize);
    
        // Now transcode the level to raw DXTn
        void *dest = (void*)(&ddsData[0] + writePos);
        if (!crnd::crnd_unpack_level(crnContext, &dest, faceSize, rowPitch, iLevel))
        {
            ddsData.clear();
            break;
        }
        writePos += faceSize;
    }
    crnd::crnd_unpack_end(crnContext);
    
    if (ddsData.size() == 0)
    {
        LogError("CRN uncompression failed!");
        return false;
    }
    return true;
}

bool TextureAsset::DeserializeFromData(const u8 *data, size_t numBytes, bool allowAsynchronous)
{
    if (assetAPI->GetFramework()->HasCommandLineParameter("--notextures"))
    {
        assetAPI->AssetLoadCompleted(Name());
        return true;
    }
    
    PROFILE(TextureAsset_DeserializeFromData);
    
    /// Force an unload of this data first.
    Unload();

    // A NullAssetFactory has been registered on headless mode.
    // We should never be here in headless mode.
    assert(!assetAPI->IsHeadless());

    allowAsynchronous &= AllowAsyncLoading();
    
    QString cacheDiskSource;
    if (allowAsynchronous)
    {
        cacheDiskSource = assetAPI->Cache()->FindInCache(Name());
        if (cacheDiskSource.isEmpty())
            allowAsynchronous = false;
    }

    QString nameSuffix = NameSuffix();
    bool isCompressed = nameSuffix == "crn" || nameSuffix == "dds";
    
    // Check if this is a crunch library CRN file and we need to decompress to DDS.
    std::vector<u8> crnUncompressData;
    if (nameSuffix == "crn")
    {
        /** If asynchronous loading is allowed we want to store the decompressed DDS data to the asset cache.
            This way below threaded loading can be done on the DDS disk source. If saving to disk fails, it is not
            fatal, we can still continue with async loading withe data ptr and len. Checking for allowAsynchronous
            also filters out any local:// etc. refs that are not meant to be loaded from asynch from asset cache.

            - Do not rewrite dds to disk if the source type for this asset is cache and the dds already exists. 
              Otherwise we would save the potentially big dds disk file every time this .crn loads!
            - If async loading is allowed and we have a valid disk source, don't do in memory decompression.
              Its a waste of resources as our disk source is up to date and we are allowed to use it to load
              into Ogre. 
        */
        QString nameInternal = NameInternal();
        cacheDiskSource = assetAPI->GetAssetCache()->FindInCache(nameInternal);
        if (allowAsynchronous)
        {
            // Only decompress and store dds if the data is new or not in cache.
            if (diskSourceType == IAsset::Original || cacheDiskSource.isEmpty())
            {
                // Input data ptr can be empty if it has been detected that we can load 
                // asynchronously, meaning there was no need to load the file data.
                std::vector<u8> fileData;
                if (!data || numBytes == 0)
                {
                    bool success = LoadFileToVector(assetAPI->GetAssetCache()->FindInCache(Name()), fileData);
                    if (!success)
                        return false;
                    data = &fileData[0];
                    numBytes = fileData.size();
                }
                if (!DecompressCRNtoDDS(data, numBytes, crnUncompressData))
                    return false;
                if (crnUncompressData.size() == 0)
                    return false;
                PROFILE(TextureAsset_DeserializeFromData_CRN_CacheStore);
                cacheDiskSource = assetAPI->GetAssetCache()->StoreAsset(&crnUncompressData[0], crnUncompressData.size(), nameInternal);
                fileData.clear();
                ELIFORP(TextureAsset_DeserializeFromData_CRN_CacheStore);
            }
            allowAsynchronous = !cacheDiskSource.isEmpty();
            if (!allowAsynchronous)
                LogWarning("TextureAsset::DeserializeFromData: Could not store decompressed CRN to asset cache, threaded loading disabled.");
        }
        if (!allowAsynchronous)
        {
            // We are doing a synchronous loading, ddsData memory is released once its loaded to ogre.
            if (crnUncompressData.size() == 0)
                if (!DecompressCRNtoDDS(data, numBytes, crnUncompressData))
                    return false;
            if (crnUncompressData.size() == 0)
                return false;
            data = (const u8*)&crnUncompressData[0];
            numBytes = crnUncompressData.size();
        }
        else
        {
            // We are bound to do threaded loading from disk.
            crnUncompressData.clear();
        }
    }
    
    // Asynchronous loading
    // 1. AssetAPI allows a asynch load. This is false when called from LoadFromFile(), LoadFromCache() etc.
    // 2. We have a rendering window for Ogre as Ogre::ResourceBackgroundQueue does not work otherwise. Its not properly initialized without a rendering window.
    // 3. The Ogre we are building against has thread support.
    if (allowAsynchronous)
    {
        // We can only do threaded loading from disk, and not any disk location but only from asset cache.
        // local:// refs will return empty string here and those will fall back to the non-threaded loading.
        // Do not change this to do DiskCache() as that directory for local:// refs will not be a known resource location for ogre.
        QFileInfo fileInfo(cacheDiskSource);
        std::string sanitatedAssetRef = fileInfo.fileName().toStdString();
        loadTicket_ = Ogre::ResourceBackgroundQueue::getSingleton().load(Ogre::TextureManager::getSingleton().getResourceType(),
                          sanitatedAssetRef, OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP, false, 0, 0, this);
        return true;
    }

    if (!data)
    {
        LogError("TextureAsset::DeserializeFromData failed: Cannot deserialize from input null pointer!");
        return false;
    }
    if (numBytes == 0)
    {
        LogError("TextureAsset::DeserializeFromData failed: numBytes == 0!");
        return false;
    }

    // Synchronous loading
    try
    {
#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)data, numBytes, false));
        
        std::vector<u8> modifiedDDSData;
        // Resize DDS images here if necessary
        if (isCompressed)
            ProcessDDSImage(stream, modifiedDDSData);
        
#include "EnableMemoryLeakCheck.h"
        // Load up the image as an Ogre CPU image object.
        Ogre::Image image;
        image.load(stream);

        // Resize non-DDS images here if necessary
        if (!isCompressed)
        {
            size_t outWidth, outHeight;
            CalculateTextureSize(image.getWidth(), image.getHeight(), outWidth, outHeight, 4*8); // Assume RGBA
            if (outWidth != image.getWidth() || outHeight != image.getHeight())
            {
                try
                {
                    LogDebug("Resizing image from " + QString::number(image.getWidth()) + "x" + QString::number(image.getHeight()) + " to " + QString::number(outWidth) + "x" + QString::number(outHeight));
                    image.resize((ushort)outWidth, (ushort)outHeight);
                }
                catch (Ogre::Exception& e)
                {
                    LogError("TextureAsset::DeserializeFromData: Failed to resize image " + Name().toStdString() + ": " + std::string(e.what()));
                }
            }
        }

        // Internal name that will be passed to Ogre for creating and loading the texture.
        // This differs from Name() only if the data was pre-processed above, eg. CRN files.
        const QString nameInternal = NameInternal();

        // If we are submitting a .dds file which did not contain mip maps, don't have Ogre generating them either.
        // Reasons:
        // 1. Not all textures need mipmaps, i.e. if the texture is always shown with 1:1 texel-to-pixel ratio, then the mip levels are never needed.
        // 2. Ogre has a bug on Apple, that it fails to generate mipmaps for .dds files which contain only one mip level and are DXT1-compressed (it tries to autogenerate, but always results in black texture data)
        // 3. If the texture is updated dynamically, we might not afford to regenerate mips at each update.
        size_t numMipmapsInImage = image.getNumMipmaps(); // Note: This is actually numMipmaps - 1: Ogre doesn't think the first level is a mipmap.
        int numMipmapsToUseOnGPU = (int)Ogre::MIP_DEFAULT;
        if (numMipmapsInImage == 0 && nameInternal.endsWith(".dds", Qt::CaseInsensitive))
            numMipmapsToUseOnGPU = 0;

        if (ogreTexture.isNull()) // If we are creating this texture for the first time, create a new Ogre::Texture object.
        {
            ogreAssetName = AssetAPI::SanitateAssetRef(nameInternal);
            
            // Optionally load textures to default pool for memory use debugging. Do not use in production use due to possible crashes on device loss & missing mipmaps!
            // Note: this does not affect async loading path, so specify additionally --noAsyncAssetLoad to be sure textures are loaded through this path
            // Furthermore, it may still allocate virtual memory address space due to using AGP memory mapping (we would not actually need a dynamic texture, but there's no way to tell Ogre that)
            if (assetAPI->GetFramework()->HasCommandLineParameter("--d3ddefaultpool"))
            {
                ogreTexture = Ogre::TextureManager::getSingleton().createManual(ogreAssetName.toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    Ogre::TEX_TYPE_2D, (Ogre::uint)image.getWidth(), (Ogre::uint)image.getHeight(), numMipmapsToUseOnGPU, image.getFormat(), Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
                ogreTexture->loadImage(image);
            }
            else
            {
                ogreTexture = Ogre::TextureManager::getSingleton().loadImage(ogreAssetName.toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, image,
                    Ogre::TEX_TYPE_2D, numMipmapsToUseOnGPU);
            }
        }
        else
        {
            // If we're loading on top of an Ogre::Texture we've created before, don't lose the old Ogre::Texture object, but reuse the old.
            // This will allow all existing materials to keep referring to this texture, and they'll get the updated texture image immediately.
            ogreTexture->freeInternalResources(); 

            if (image.getWidth() != ogreTexture->getWidth() || image.getHeight() != ogreTexture->getHeight() || image.getFormat() != ogreTexture->getFormat())
            {
                ogreTexture->setWidth(image.getWidth());
                ogreTexture->setHeight(image.getHeight());
                ogreTexture->setFormat(image.getFormat());
            }

            if (ogreTexture->getBuffer().isNull())
            {
                LogError("DeserializeFromData: Failed to create texture " + this->Name() + ": OgreTexture::getBuffer() was null!");
                return false;
            }

            Ogre::PixelBox pixelBox(Ogre::Box(0,0, image.getWidth(), image.getHeight()), image.getFormat(), (void*)image.getData());
            ogreTexture->getBuffer()->blitFromMemory(pixelBox);

            ogreTexture->createInternalResources();
        }

        if (NameSuffix() == "crn" && crnUncompressData.size() > 0)
            crnUncompressData.clear();

        PostProcessTexture();
        
        // We did a synchronous load and must call AssetLoadCompleted here.
        // This is done with Name() that is tracked by AssetAPI and is the ref
        // we are showing outside this object, even if the input data was pre-processed 
        // before passed to Ogre with NameInternal().
        assetAPI->AssetLoadCompleted(Name());
        return true;
    }
    catch(Ogre::Exception &e)
    {
        LogError("TextureAsset::DeserializeFromData: Failed to create texture " + Name().toStdString() + ": " + std::string(e.what()));
        return false;
    }
}

void TextureAsset::operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result)
{
    if (ticket != loadTicket_)
        return;
        
    // Reset to 0 to mark the asynch request is not active anymore. Aborted in Unload() if it is.
    loadTicket_ = 0;

    ogreAssetName = AssetAPI::SanitateAssetRef(NameInternal());
    if (!result.error)
    {
        ogreTexture = Ogre::TextureManager::getSingleton().getByName(ogreAssetName.toStdString(), OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP);
        if (!ogreTexture.isNull())
        {
            PostProcessTexture();
            
            assetAPI->AssetLoadCompleted(Name());
            return;
        }
        else
            LogError("TextureAsset asynch load: Ogre::Texture was null after threaded loading: " + Name());
    }
    else
        LogError("TextureAsset asynch load: Ogre failed to do threaded loading: " + result.message);

    DoUnload();
    assetAPI->AssetLoadFailed(Name());
}

/*
void TextureAsset::RegenerateAllMipLevels()
{
    if (ogreTexture.isNull())
        return;

///\todo This function does not quite work, since ogreTexture->getNumMipmaps() will return 0 to denote a "full mipmap chain".

    for(int f = 0; f < ogreTexture->getNumFaces(); ++f)
        for(int i = 1; i < ogreTexture->getNumMipmaps(); ++i)
        {
            Ogre::HardwarePixelBufferSharedPtr src = ogreTexture->getBuffer(f, i-1);
            Ogre::Box srcSize(0, 0, src->getWidth(), src->getHeight());
            Ogre::HardwarePixelBufferSharedPtr dst = ogreTexture->getBuffer(f, i);
            Ogre::Box dstSize(0, 0, dst->getWidth(), dst->getHeight());
            dst->blit(src, srcSize, dstSize);
        }
}
*/

bool TextureAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    PROFILE(TextureAsset_SerializeTo);
    if (ogreTexture.isNull())
    {
        LogWarning("SerializeTo: Called on an unloaded texture \"" + Name() + "\".");
        return false;
    }

    try
    {
        Ogre::Image newImage;
        ogreTexture->convertToImage(newImage);
        std::string formatExtension = serializationParameters.trimmed().toStdString();
        if (formatExtension.empty())
        {
            LogDebug("TextureAsset::SerializeTo: no serializationParameters given. Trying to guess format extension from the asset name.");
            formatExtension = QFileInfo(NameInternal()).suffix().toStdString();
        }

        Ogre::DataStreamPtr imageStream = newImage.encode(formatExtension);
        if (imageStream.get() && imageStream->size() > 0)
        {
            data.resize(imageStream->size());
            imageStream->read(&data[0], data.size());
        }
    } catch(std::exception &e)
    {
        LogError("SerializeTo: Failed to export Ogre texture " + Name() + ":");
        if (e.what())
            LogError(e.what());
        return false;
    }
    return true;
}

void TextureAsset::DoUnload()
{
    // If a ongoing asynchronous asset load requested has been made to ogre, we need to abort it.
    // Otherwise Ogre will crash to our raw pointer that was passed if we get deleted. A ongoing ticket id cannot be 0.
    if (loadTicket_ != 0)
    {
        Ogre::ResourceBackgroundQueue::getSingleton().abortRequest(loadTicket_);
        loadTicket_ = 0;
    }
    
    if (!ogreTexture.isNull())
        ogreAssetName = ogreTexture->getName().c_str();

    ogreTexture = Ogre::TexturePtr();
    try
    {
        Ogre::TextureManager::getSingleton().remove(ogreAssetName.toStdString());
    }
    catch(...) {}
}

bool TextureAsset::IsLoaded() const
{
    return ogreTexture.get() != 0;
}

QImage TextureAsset::ToQImage(Ogre::Texture* tex, size_t faceIndex, size_t mipmapLevel)
{
    PROFILE(TextureAsset_ToQImage);
    if (!tex)
    {
        LogError("TextureAsset::ToQImage: Can't convert texture to QImage, null texture pointer");
        return QImage();
    }
    
    Ogre::Image ogreImage;
    tex->convertToImage(ogreImage);
    QImage::Format fmt;
    switch(ogreImage.getFormat())
    {
    case Ogre::PF_X8R8G8B8: fmt = QImage::Format_RGB32; break;
    case Ogre::PF_A8R8G8B8: fmt = QImage::Format_ARGB32; break;
    case Ogre::PF_R5G6B5: fmt = QImage::Format_RGB16; break;
    case Ogre::PF_R8G8B8: fmt = QImage::Format_RGB888; break;
    default:
        LogError("TextureAsset::ToQImage: Can't convert texture " + QString::fromStdString(tex->getName()) + " to QImage, unsupported image format " + QString::number(ogreImage.getFormat()));
        return QImage();
    }

    QImage img((int)ogreImage.getWidth(), (int)ogreImage.getHeight(), fmt);
    assert(img.byteCount() == (int)ogreImage.getSize());
    memcpy(img.bits(), ogreImage.getData(), img.byteCount());

    return img;
}

QImage TextureAsset::ToQImage(size_t faceIndex, size_t mipmapLevel) const
{
    if (!ogreTexture.get())
    {
        LogError("TextureAsset::ToQImage: Can't convert texture to QImage, Ogre texture is not initialized for asset \"" + ToString() + "\"!");
        return QImage();
    }
    
    return ToQImage(ogreTexture.get(), faceIndex, mipmapLevel);
}

size_t TextureAsset::Height() const
{
    if (!ogreTexture.get())
        return 0;
    return ogreTexture->getHeight();
}

size_t TextureAsset::Width() const
{
    if (!ogreTexture.get())
        return 0;
    return ogreTexture->getWidth();
}

void TextureAsset::SetContentsFillSolidColor(int newWidth, int newHeight, u32 color, Ogre::PixelFormat ogreFormat, bool regenerateMipmaps, bool dynamic)
{
    if (newWidth == 0 || newHeight == 0)
    {
        Unload();
        return;
    }
    ///\todo Could optimize a lot here, don't create this temporary vector.
    ///\todo This only works for 32bpp images.
    std::vector<u32> data(newWidth * newHeight, color);
    SetContents(newWidth, newHeight, (const u8*)&data[0], data.size() * sizeof(u32), ogreFormat, regenerateMipmaps, dynamic, false);
}

void TextureAsset::SetContents(size_t newWidth, size_t newHeight, const u8 *data, size_t numBytes, Ogre::PixelFormat ogreFormat, bool regenerateMipMaps, bool dynamic, bool renderTarget)
{
    PROFILE(TextureAsset_SetContents);

    int usage = dynamic ? Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE : Ogre::TU_STATIC_WRITE_ONLY;
    if (regenerateMipMaps)
        usage |= Ogre::TU_AUTOMIPMAP;
    if (renderTarget)
        usage |= Ogre::TU_RENDERTARGET;

    if (numBytes != newWidth * newHeight * 4)
    {
        LogError("TextureAsset::SetContents failed: Inputted " + QString::number(numBytes) + " bytes of data, but " + QString::number(newWidth) + "x" + QString::number(newHeight)
            + " at 4 bytes per pixel requires " + QString::number(newWidth * newHeight * 4) + " bytes!");
        return;
    }

    if (!ogreTexture.get())
    {
        ogreTexture = Ogre::TextureManager::getSingleton().createManual(NameInternal().toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D,
            (Ogre::uint)newWidth, (Ogre::uint)newHeight, regenerateMipMaps ? Ogre::MIP_UNLIMITED : 0, ogreFormat, usage);
        if (!ogreTexture.get())
        {
            LogError("TextureAsset::SetContents failed: Cannot create texture asset \"" + ToString() + "\" to name \"" + Name() + "\" and size " + QString::number(newWidth) + "x" + QString::number(newHeight) + "!");
            return;
        }
    }

    bool needRecreate = (newWidth != ogreTexture->getWidth() || newHeight != ogreTexture->getHeight() || ogreFormat != ogreTexture->getFormat());
//    if (newWidth == ogreTexture->getWidth() && newHeight == ogreTexture->getHeight() && ogreFormat == ogreTexture->getFormat())
//        return;

    if (needRecreate)
    {
        ogreTexture->freeInternalResources(); 
        ogreTexture->setWidth(newWidth);
        ogreTexture->setHeight(newHeight);
        ogreTexture->setFormat(ogreFormat);
        ogreTexture->createInternalResources();
    }
    if (ogreTexture->getBuffer().isNull())
    {
        LogError("TextureAsset::SetContents: Failed to create texture " + this->Name() + ": OgreTexture::getBuffer() was null!");
        return;
    }

    if (data)
    {
/*
#if defined(DIRECTX_ENABLED) && defined(WIN32)
        Ogre::HardwarePixelBufferSharedPtr pb = ogreTexture->getBuffer();
        Ogre::D3D9HardwarePixelBuffer *pixelBuffer = dynamic_cast<Ogre::D3D9HardwarePixelBuffer*>(pb.get());
        assert(pixelBuffer);

        LPDIRECT3DSURFACE9 surface = pixelBuffer->getSurface(Ogre::D3D9RenderSystem::getActiveD3D9Device());
        if (surface)
        {
            D3DSURFACE_DESC desc;
            HRESULT hr = surface->GetDesc(&desc);
            if (SUCCEEDED(hr))
            {
                D3DLOCKED_RECT lock;
                HRESULT hr = surface->LockRect(&lock, 0, 0);
                if (SUCCEEDED(hr))
                {
                    const int bytesPerPixel = 4; ///\todo Count from Ogre::PixelFormat!
                    const int sourceStride = bytesPerPixel * newWidth;
                    if (lock.Pitch == sourceStride)
                        memcpy(lock.pBits, data, sourceStride * newHeight);
                    else
                        for(size_t y = 0; y < newHeight; ++y)
                            memcpy((u8*)lock.pBits + lock.Pitch * y, data + sourceStride * y, sourceStride);
                    surface->UnlockRect();
                }
            }
        }
#else        
        */
        ///\todo Review Ogre internals of whether the const_cast here is safe!
        Ogre::PixelBox pixelBox(Ogre::Box(0,0, newWidth, newHeight), ogreFormat, const_cast<u8*>(data));
        ogreTexture->getBuffer()->blitFromMemory(pixelBox);
//#endif
    }
}

void TextureAsset::SetContentsDrawText(int newWidth, int newHeight, QString text, const QColor &textColor, const QFont &font, const QBrush &backgroundBrush, const QPen &borderPen, int flags, bool generateMipmaps, bool dynamic,
                                       float xRadius, float yRadius)
{
    PROFILE(TextureAsset_SetContentsDrawText);
    text = text.replace("\\n", "\n");

    // Create transparent pixmap
    QImage image(newWidth, newHeight, QImage::Format_ARGB32);
    image.fill(textColor.rgb() & 0x00FFFFFF);

    {
        // Init painter with pixmap as the paint device
        QPainter painter(&image);

        // Ask painter the rect for the text
        painter.setFont(font);
        QRect rect = painter.boundingRect(image.rect(), flags, text);

        // Set background brush
        painter.setBrush(backgroundBrush);
        painter.setPen(borderPen);
  
        painter.drawRoundedRect(rect, xRadius, yRadius, Qt::RelativeSize);
        
        // Draw text
        painter.setPen(textColor);
        painter.drawText(rect, flags, text);
    }

    SetContents(newWidth, newHeight, image.bits(), image.byteCount(), Ogre::PF_A8R8G8B8, generateMipmaps, dynamic, false);
}

void TextureAsset::PostProcessTexture()
{
    if (assetAPI->GetFramework()->HasCommandLineParameter("--autoDxtCompress"))
        CompressTexture();
}

void TextureAsset::CompressTexture()
{
#if defined(DIRECTX_ENABLED) && defined(WIN32)
    if (ogreTexture.isNull())
        return;
    
    QStringList sizeParam = assetAPI->GetFramework()->CommandLineParameters("--maxTextureSize");
    size_t maxTextureSize = 0;
    if (sizeParam.size() > 0)
    {
        int size = sizeParam.first().toInt();
        if (size > 0)
            maxTextureSize = size;
    }
    
    Ogre::PixelFormat sourceFormat = ogreTexture->getFormat();
    if (sourceFormat >= Ogre::PF_DXT1 && sourceFormat <= Ogre::PF_DXT5)
        return; // Already compressed, do nothing
    if ((sourceFormat >= Ogre::PF_L8 && sourceFormat <= Ogre::PF_BYTE_LA) || sourceFormat == Ogre::PF_R8)
        return; // 1 or 2 byte format, leave alone
    
    PROFILE(TextureAsset_CompressTexture);
    
    // Ogre will crash on OpenGL if it tries to get texture data. Therefore perform a no-op on OpenGL
    /// \todo Fix the crash
    // (note: on OpenGL memory use of the Tundra process is less critical anyway, as the system memory copy
    // of the texture is contained by the display driver)
    if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getName() == "OpenGL Rendering Subsystem")
    {
        LogWarning("Skipping CompressTexture on OpenGL as it is prone to crash");
        return;
    }
    
    // Get original texture data
    std::vector<unsigned char*> imageData;
    std::vector<Ogre::PixelBox> imageBoxes;
    
    size_t numMipmaps = ogreTexture->getNumMipmaps();
    
    for (size_t level = 0; level <= numMipmaps; ++level)
    {
        try
        {
            Ogre::HardwarePixelBufferSharedPtr buf = ogreTexture->getBuffer(0, level);
            
            // If a max texture size is set, and mipmaps are present in the source texture, reject those larger than acceptable texture size, however ensure at least 1 mipmap
            if (maxTextureSize > 0 && numMipmaps > 0)
            {
                if (level < numMipmaps && imageBoxes.size() == 0 && (buf->getWidth() > maxTextureSize || buf->getHeight() > maxTextureSize))
                    continue;
            }
            
            unsigned char* levelData = new unsigned char[buf->getWidth() * buf->getHeight() * 4];
            imageData.push_back(levelData);
            Ogre::PixelBox levelBox(Ogre::Box(0, 0, buf->getWidth(), buf->getHeight()), Ogre::PF_A8B8G8R8, levelData);
            buf->blitToMemory(levelBox);
            imageBoxes.push_back(levelBox);
        }
        catch (std::exception& e)
        {
            LogError("TextureAsset::CompressTexture: Caught exception " + QString(e.what()) + " while handling miplevel " + QString::number(level) + ", aborting.");
            break;
        }
    }
    
    // If we only have 1 mipmap, and it is too large, resample it now
    if (maxTextureSize > 0 && imageBoxes.size() == 1 && (imageBoxes[0].right > maxTextureSize || imageBoxes[0].bottom > maxTextureSize))
    {
        size_t targetWidth = imageBoxes[0].right;
        size_t targetHeight = imageBoxes[0].bottom;
        while (targetWidth > maxTextureSize || targetHeight > maxTextureSize)
        {
            targetWidth >>= 1;
            targetHeight >>= 1;
        }
        if (!targetWidth)
            targetWidth = 1;
        if (!targetHeight)
            targetHeight = 1;
        
        unsigned char* scaledPixelData = new unsigned char[targetWidth * targetHeight * 4];
        Ogre::PixelBox targetBox(Ogre::Box(0, 0, targetWidth, targetHeight), Ogre::PF_A8B8G8R8, scaledPixelData);
        Ogre::Image::scale(imageBoxes[0], targetBox);
        
        // Delete the unscaled original data and replace the original pixelbox
        delete[] imageData[0];
        imageData[0] = scaledPixelData;
        imageBoxes[0] = targetBox;
    }
    
    // Determine format
    int flags = squish::kColourRangeFit; // Lowest quality, but fastest
    size_t bytesPerBlock = 8;
    Ogre::PixelFormat newFormat = Ogre::PF_DXT1;
    if (ogreTexture->hasAlpha())
    {
        LogDebug("CompressTexture " + Name() + " image format " + QString::number(sourceFormat) + ", compressing as DXT5");
        newFormat = Ogre::PF_DXT5;
        bytesPerBlock = 16;
        flags |= squish::kDxt5;
    }
    else
    {
        LogDebug("CompressTexture " + Name() + " image format " + QString::number(sourceFormat) + ", compressing as DXT1");
        flags |= squish::kDxt1;
    }
    
    // Compress original texture data
    std::vector<unsigned char*> compressedImageData;
    
    for (size_t level = 0; level < imageBoxes.size(); ++level)
    {
        int compressedSize = squish::GetStorageRequirements((int)imageBoxes[level].right, (int)imageBoxes[level].bottom, flags);
        LogDebug("Compressing level " + QString::number(level) + " " + QString::number(imageBoxes[level].right) + "x" + QString::number(imageBoxes[level].bottom) + " into " + QString::number(compressedSize) + " bytes");
        unsigned char* compressedData = new unsigned char[compressedSize];
        squish::CompressImage((squish::u8*)imageData[level], (int)imageBoxes[level].right, (int)imageBoxes[level].bottom, compressedData, flags);
        compressedImageData.push_back(compressedData);
    }
    
    // Change Ogre texture format
    ogreTexture->freeInternalResources();
    ogreTexture->setWidth(imageBoxes[0].right);
    ogreTexture->setHeight(imageBoxes[0].bottom);
    ogreTexture->setFormat(newFormat);
    ogreTexture->setNumMipmaps(imageBoxes.size() - 1);
    ogreTexture->createInternalResources();
    
    // Upload compressed texture data
    for (size_t level = 0; level < imageBoxes.size(); ++level)
    {
        try
        {
            Ogre::HardwarePixelBufferSharedPtr buf = ogreTexture->getBuffer(0, level);
            
            // Ogre does not load the texture data properly if the miplevel width is not divisible by 4, so we write manual code for Direct3D9
            /// \todo Fix bug in ogre-safe-nocrashes branch. It also affects Ogre's loading of already compressed DDS files
            
            size_t numRows = (buf->getHeight() + 3) / 4;
            size_t sourceStride = (buf->getWidth() + 3) / 4 * bytesPerBlock;
            unsigned char* src = compressedImageData[level];
            
            Ogre::D3D9HardwarePixelBuffer *pixelBuffer = dynamic_cast<Ogre::D3D9HardwarePixelBuffer*>(buf.get());
            assert(pixelBuffer);
            LPDIRECT3DSURFACE9 surface = pixelBuffer->getSurface(Ogre::D3D9RenderSystem::getActiveD3D9Device());
            if (surface)
            {
                D3DLOCKED_RECT lock;
                HRESULT hr = surface->LockRect(&lock, 0, 0);
                if (SUCCEEDED(hr))
                {
                    if ((size_t)lock.Pitch == sourceStride)
                        memcpy(lock.pBits, src, sourceStride * numRows);
                    else
                        for(size_t y = 0; y < numRows; ++y)
                            memcpy((u8*)lock.pBits + lock.Pitch * y, src + sourceStride * y, sourceStride);
                    surface->UnlockRect();
                }
            }
        }
        catch (std::exception& e)
        {
            LogError("TextureAsset::CompressTexture: Caught exception " + QString(e.what()) + " while handling miplevel " + QString::number(level) + ", aborting.");
            break;
        }
    }
    
    // Delete CPU-side temp image data
    for (size_t i = 0; i < imageData.size(); ++i)
        delete[] imageData[i];
    for (size_t i = 0; i < compressedImageData.size(); ++i)
        delete[] compressedImageData[i];
#endif
}

bool TextureAsset::AllowAsyncLoading() const
{
    /// \todo NeedSizeModification() does not take into account the current texture's data size, in which case we may go on the threaded loading path
    /// without a possibility to resize the texture smaller. This means that potentially one texture may go in unresized and increase the texture load
    /// significantly over the budget.
    if (NeedSizeModification() || assetAPI->GetFramework()->IsHeadless() || assetAPI->GetFramework()->HasCommandLineParameter("--no_async_asset_load") ||
        assetAPI->GetFramework()->HasCommandLineParameter("--noAsyncAssetLoad") || !assetAPI->GetAssetCache() || (OGRE_THREAD_SUPPORT == 0))
        return false;
    else
        return true;
}

bool TextureAsset::NeedSizeModification() const
{
    OgreRenderer::RendererPtr renderer = assetAPI->GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    return assetAPI->GetFramework()->HasCommandLineParameter("--maxTextureSize") || renderer->TextureBudgetUse() > BUDGET_THRESHOLD || 
        renderer->TextureQuality() == OgreRenderer::Renderer::Texture_Low;
}

void TextureAsset::CalculateTextureSize(size_t width, size_t height, size_t& outWidth, size_t& outHeight, size_t bitsPerPixel)
{
    OgreRenderer::RendererPtr renderer = assetAPI->GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();

    outWidth = width;
    outHeight = height;
    
    if (renderer->TextureQuality() == OgreRenderer::Renderer::Texture_Low)
    {
        outWidth >>= 1;
        outHeight >>= 1;
    }
    
    float t = renderer->TextureBudgetUse(outWidth * outHeight * bitsPerPixel / 8);
    if (t > BUDGET_THRESHOLD)
    {
        for (;;)
        {
            size_t maxTextureSize = 4096;
            int factor = (int)((t - BUDGET_THRESHOLD) / BUDGET_STEP);
            if (factor < 0)
                factor = 0;
            maxTextureSize >>= factor;
            if (!maxTextureSize)
                maxTextureSize = 1;

            if (outWidth <= maxTextureSize && outHeight <= maxTextureSize)
                break;
            
            outWidth >>= 1;
            outHeight >>= 1;

            // Update the tentative budget and check whether further reduction is needed
            t = renderer->TextureBudgetUse(outWidth * outHeight * bitsPerPixel / 8);
        }
    }
    
    if (assetAPI->GetFramework()->HasCommandLineParameter("--maxTextureSize"))
    {
        QStringList sizeParam = assetAPI->GetFramework()->CommandLineParameters("--maxTextureSize");
        size_t maxTextureSize = 0;
        if (sizeParam.size() > 0)
        {
            int size = sizeParam.first().toInt();
            if (size > 0)
                maxTextureSize = size;
        }
        if (maxTextureSize)
        {
            while (outWidth > maxTextureSize || outHeight > maxTextureSize)
            {
                outWidth >>= 1;
                outHeight >>= 1;
            }
        }
    }
    
    if (!outWidth)
        outWidth = 1;
    if (!outHeight)
        outHeight = 1;
}

#include "DisableMemoryLeakCheck.h"

// Code from http://www.ogre3d.org/forums/viewtopic.php?f=4&t=50282&hilit=gpu+memory+mipmap#p342476

/** Removes top MipMap levels from a dds file (stream)
   @remarks This function loads an image file from a stream,
       if the file is a dds file that has MipMaps -
       the number of top MipMap level that are defined
       by a parameter will be removed
   @param stream - source stream with the image file data
   @param numberOfTopMipMapToSkip - number of top MipMap levels that will be removed
   @returns - a self releasing memory stream with the modified dds
   */
void TextureAsset::ProcessDDSImage(Ogre::DataStreamPtr& stream, std::vector<u8>& modifiedDDSData)
{
    // simplified dds header struct with the minimum data needed
    struct DDSHEADER {
        char cMagic[4];
        unsigned dwSize;
        unsigned dwFlags;
        unsigned dwHeight;
        unsigned dwWidth;
        unsigned dwPitchOrLinearSize;
        unsigned dwDepth;
        unsigned dwMipMapCount;
        unsigned dwReserved1[11];
        unsigned dwSizeOfPixelFormat;
        unsigned dwFlagsOfPixelFormat;
        char cFourCharIdOfPixelFormat[4];
        char dataThatWeDoNotNeed[40];
    };

    // return to the stream beginning 
    stream->seek(0);

    // read the header
    DDSHEADER header;
    stream->read(&header, sizeof(DDSHEADER));

    // check if this is a valid dds file by the image type id
    bool isMagicValid = ( memcmp(header.cMagic, "DDS ", 4) == 0);

    // check if the pixel type is DXT (do the check only if the magic is valid...)
    bool isDXT = isMagicValid && memcmp(header.cFourCharIdOfPixelFormat, "DXT", 3) == 0;
    bool isDXT1 = memcmp(header.cFourCharIdOfPixelFormat, "DXT1", 4) == 0;
    
    // check if this is a valid dds file by the image type id
    bool isValidDdsFile = isMagicValid && isDXT;

    long totalSizeOfTheSkipTopLevels = 0;

    size_t outWidth, outHeight;
    CalculateTextureSize(header.dwWidth, header.dwHeight, outWidth, outHeight, isDXT1 ? 4 : 8);

    if (outWidth == header.dwWidth && outHeight == header.dwHeight)
    {
        stream->seek(0);
        return; // No resize, can use original stream
    }

    size_t numberOfTopMipMapToSkip = 0;
    size_t curWidth = header.dwWidth;
    size_t curHeight = header.dwHeight;
    while (curWidth > outWidth || curHeight > outHeight)
    {
        // Do not allow to go below 4 in either dimension
        if (curWidth == 4 || curHeight == 4)
            break;
        curWidth >>= 1;
        curHeight >>= 1;
        ++numberOfTopMipMapToSkip;
    }
    
    // If no mips, can not resize
    if (!header.dwMipMapCount || header.dwMipMapCount == 1)
        numberOfTopMipMapToSkip = 0;
    else if (numberOfTopMipMapToSkip > header.dwMipMapCount - 1)
        numberOfTopMipMapToSkip = header.dwMipMapCount - 1;
    
    if (!numberOfTopMipMapToSkip)
    {
        stream->seek(0);
        return; // Can use original stream
    }

    if (isValidDdsFile)
    {
        // store the width and height as local vars for easy access 
        unsigned long width = header.dwWidth;
        unsigned long height = header.dwHeight;
        unsigned long mipMapCount = header.dwMipMapCount;

        // skip the levels (if has MipMap and a valid file)
        for (size_t i = 0 ; i < numberOfTopMipMapToSkip && mipMapCount > 1 && isValidDdsFile ; i++)
        {
            // calculate the size current top level
            long sizeOfCurTopLevel = width  * height;

            // if the pixel type is DXT1 - the size is half of DXT3 or DXT5
            if (isDXT1)
                sizeOfCurTopLevel /= 2;

            // skip the current top level
            totalSizeOfTheSkipTopLevels += sizeOfCurTopLevel;
        
            // update to the new size
            height /= 2;
            width /= 2;

            // decrement the MipMap count by one  
            mipMapCount -= 1; 
        }

        // change the header to be without the top levels that were removed
        header.dwHeight = height;
        header.dwWidth = width;
        header.dwMipMapCount = mipMapCount; 

        // skip the top levels in the stream
        stream->skip(totalSizeOfTheSkipTopLevels);
    }
    else
    {
        LogDebug("TextureAsset::ProcessDDSImage: texture " + Name() + " does not contain DXT compressed data, skipping resize");
        stream->seek(0);
        return; // Data could not be processed, use original
    }

    LogDebug("TextureAsset::ProcessDDSImage: resizing texture " + Name() + " from " + QString::number(header.dwWidth) + "x" + QString::number(header.dwHeight) + " to " + QString::number(curWidth) + "x" + QString::number(curHeight));

    // create the memory for the loaded data
    size_t sizeOfTheDddWithoutTopLevels = 
        stream->size() - totalSizeOfTheSkipTopLevels;

    modifiedDDSData.resize(sizeOfTheDddWithoutTopLevels);

    // add the header
    memcpy(&modifiedDDSData[0], &header, sizeof(DDSHEADER));

    // read the rest of the data
    size_t sizeOfTheDddsWithoutTopLevelsAndHeaders 
        = sizeOfTheDddWithoutTopLevels - sizeof(DDSHEADER);

    stream->read(&modifiedDDSData[sizeof(DDSHEADER)], 
        sizeOfTheDddsWithoutTopLevelsAndHeaders);

    // Overwrite stream with new
    stream = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(&modifiedDDSData[0], sizeOfTheDddWithoutTopLevels, false));
}

#include "EnableMemoryLeakCheck.h"
