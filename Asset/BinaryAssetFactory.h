// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_BinaryAssetFactory_h
#define incl_Asset_BinaryAssetFactory_h

#include "IAssetTypeFactory.h"
#include "BinaryAsset.h"

class BinaryAssetFactory : public IAssetTypeFactory
{
    Q_OBJECT;
public:
    explicit BinaryAssetFactory(const char *assetType_)
    {
        assert(assetType_ && "Must specify an asset type for asset factory!");
        assetType = assetType_;
        assetType = assetType.trimmed();
        assert(!assetType.isEmpty() && "Must specify an asset type for asset factory!");
    }

    virtual QString Type() const { return assetType; }

    virtual AssetPtr CreateEmptyAsset(const char *name) { return AssetPtr(new BinaryAsset(Type(), name)); }

private:
    QString assetType;
};

#endif
