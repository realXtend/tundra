// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "IAssetTypeFactory.h"

/// A factory that always returns a null pointer for creating assets.
/** This factory is used to ignore certain asset types when they are supposed to never be used in the system.
    Another way to disable the use of certain assets is to not register a factory at all for that type.
    However, that will log out an error message. Instead, by using the Null Factory we can signal that 
    we are ok that the assets of the given type are not loaded. */
class TUNDRACORE_API NullAssetFactory : public IAssetTypeFactory
{
public:
    explicit NullAssetFactory(const QString &assetType_, const QString &assetTypeExtension) :
        assetType(assetType_.trimmed()),
        assetTypeExtensions(assetTypeExtension)
    {
        assert(!assetType.isEmpty() && "Must specify an asset type for null asset factory!");
        assert(!assetTypeExtension.trimmed().isEmpty() && "Asset type extension cannot be empty for null asset factory!");
    }
    
    explicit NullAssetFactory(const QString &assetType_, const QStringList &assetTypeExtensions_) :
        assetType(assetType_.trimmed()),
        assetTypeExtensions(assetTypeExtensions_)
    {
        assert(!assetType.isEmpty() && "Must specify an asset type for null asset factory!");
        assert(!assetTypeExtensions.isEmpty() && "Must specify at least one asset type extension for null asset factory!");
    }

    virtual const QString &Type() const { return assetType; }
    
    virtual const QStringList &TypeExtensions() const { return assetTypeExtensions; }

    virtual AssetPtr CreateEmptyAsset(AssetAPI *, const QString &) { return AssetPtr(); }

private:
    const QString assetType;
    const QStringList assetTypeExtensions;
};

