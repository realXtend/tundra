// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_GenericAssetFactory_h
#define incl_Asset_GenericAssetFactory_h

#include "IAssetTypeFactory.h"
#include "BinaryAsset.h"

/// GenericAssetFactory is a predefined concrete factory type anyone defining a new asset type can use to create new assets of any type.

template<typename AssetType> class GenericAssetFactory : public IAssetTypeFactory
{

public:
    /// GenericAssetFactory constructor.
    /// @param QString asset type eg. "Audio".
    /// @param QStringList supported file formats eg. ["wav", "mp3", "ogg"].
    explicit GenericAssetFactory(const QString assetType, const QStringList supportedFileFormats)
    {
        // Set type and validate
        assetType_ = assetType.trimmed();
        assert(!assetType_.isEmpty() && "Must specify an asset type for asset factory!");
        
        // Set formats, validate here that they are lower case.
        foreach(QString format, supportedFileFormats)
            supportedFileFormats_ << format.trimmed().toLower();
    }

    /// GenericAssetFactory constructor. Overload for factories that supports one file format.
    /// @param QString asset type eg. "Audio".
    /// @param QString supported file format "wav".
    explicit GenericAssetFactory(const QString assetType, const QString supportedFileFormat)
    {
        // Set type and validate
        assetType_ = assetType.trimmed();
        assert(!assetType_.isEmpty() && "Must specify an asset type for asset factory!");

        // Set formats, validate here that they are lower case.
        supportedFileFormats_ << supportedFileFormat.trimmed().toLower();
    }

    /// Create empty asset with name.
    /// @param AssetAPI asset api ptr.
    /// @param char* name of the created asset.
    /// @return AssetPtr empty asset.
    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const char *name) { return AssetPtr(new AssetType(owner, GetType(), name)); }
};

/// For simple asset types the client wants to parse, we define the BinaryAssetFactory type.
typedef GenericAssetFactory<BinaryAsset> BinaryAssetFactory;

#endif
