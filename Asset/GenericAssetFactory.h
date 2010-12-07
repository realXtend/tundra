// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_GenericAssetFactory_h
#define incl_Asset_GenericAssetFactory_h

#include "IAssetTypeFactory.h"
#include "BinaryAsset.h"

/// GenericAssetFactory is a predefined concrete factory type anyone defining a new asset type can use
/// to create new assets of any type.
template<typename AssetType>
class GenericAssetFactory : public IAssetTypeFactory
{
public:
    explicit GenericAssetFactory(const char *assetType_)
    {
        assert(assetType_ && "Must specify an asset type for asset factory!");
        assetType = assetType_;
        assetType = assetType.trimmed();
        assert(!assetType.isEmpty() && "Must specify an asset type for asset factory!");
    }

    virtual QString Type() const { return assetType; }

    virtual AssetPtr CreateEmptyAsset(const char *name) { return AssetPtr(new AssetType(Type(), name)); }

private:
    QString assetType;
};

/// For simple asset types the client wants to parse, we define the BinaryAssetFactory type.
typedef GenericAssetFactory<BinaryAsset> BinaryAssetFactory;

#endif
