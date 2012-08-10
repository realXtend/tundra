// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAssetTypeFactory.h"
#include "BinaryAsset.h"

#include <boost/make_shared.hpp>

/// A factory for instantiating assets of a templated type T.
/** GenericAssetFactory is a predefined concrete factory type anyone defining a new asset type can use
    to create new assets of any type. */
template<typename AssetType>
class GenericAssetFactory : public IAssetTypeFactory
{
public:
    explicit GenericAssetFactory(const QString &assetType_, const QString &assetTypeExtension)
    {
        assetType = assetType_.trimmed();
        assert(!assetType.isEmpty() && "Must specify an asset type for asset factory!");
        // assetTypeExtension can be empty in the case of BinaryAsset, don't assert it.
        assetTypeExtensions << assetTypeExtension;
    }

    explicit GenericAssetFactory(const QString &assetType_, const QStringList &assetTypeExtensions_)
    {
        assetType = assetType_.trimmed();
        assert(!assetType.isEmpty() && "Must specify an asset type for asset factory!");
        assetTypeExtensions = assetTypeExtensions_;
        assert(!assetTypeExtensions.isEmpty() && "Must specify at least one asset type extension for asset factory!");
    }

    virtual QString Type() const { return assetType; }
    
    virtual QStringList TypeExtensions() const { return assetTypeExtensions; }

    virtual AssetPtr CreateEmptyAsset(AssetAPI *owner, const QString &name) { return boost::make_shared<AssetType>(owner, Type(), name); }

private:
    QString assetType;
    QStringList assetTypeExtensions;
};

/// For simple asset types the client wants to parse, we define the BinaryAssetFactory type.
typedef GenericAssetFactory<BinaryAsset> BinaryAssetFactory;
