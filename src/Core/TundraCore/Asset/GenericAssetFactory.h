// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAssetTypeFactory.h"
#include "BinaryAsset.h"

/// A factory for instantiating assets of a templated type T.
/** GenericAssetFactory is a predefined concrete factory type anyone defining a new asset type can use
    to create new assets of any type. */
template<typename AssetType>
class GenericAssetFactory : public IAssetTypeFactory
{
public:
    explicit GenericAssetFactory(const QString &assetType_, const QString &assetTypeExtension) :
        assetType(assetType_.trimmed()),
        assetTypeExtensions(assetTypeExtension)
    {
        assert(!assetType.isEmpty() && "Must specify an asset type for asset factory!");
        // assetTypeExtension can be empty in the case of BinaryAsset, don't assert it.
    }

    explicit GenericAssetFactory(const QString &assetType_, const QStringList &assetTypeExtensions_) :
        assetType(assetType_.trimmed()),
        assetTypeExtensions(assetTypeExtensions_)
    {
        assert(!assetType.isEmpty() && "Must specify an asset type for asset factory!");
        assert(!assetTypeExtensions.isEmpty() && "Must specify at least one asset type extension for asset factory!");
    }

    virtual const QString &Type() const { return assetType; }
    
    virtual const QStringList &TypeExtensions() const { return assetTypeExtensions; }

    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const QString &name) { return MAKE_SHARED(AssetType, owner, Type(), name); }

private:
    const QString assetType;
    const QStringList assetTypeExtensions;
};

/// For simple asset types the client wants to parse, we define the BinaryAssetFactory type.
typedef GenericAssetFactory<BinaryAsset> BinaryAssetFactory;
