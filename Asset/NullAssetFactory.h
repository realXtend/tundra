// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_NullAssetFactory_h
#define incl_Asset_NullAssetFactory_h

#include "IAssetTypeFactory.h"

/// NullAssetFactory is a factory that always returns a null pointer for creating assets.
/// This factory is used to ignore certain asset types when they are supposed to never be used in the system.
/// Another way to disable the use of certain assets is to not register a factory at all for that type.
/// However, that will log out an error message. Instead, by using the Null Factory we can signal that 
/// we are ok that the assets of the given type are not loaded.
class NullAssetFactory : public IAssetTypeFactory
{
public:
    explicit NullAssetFactory(const char *assetType_)
    {
        assert(assetType_ && "Must specify an asset type for asset factory!");
        assetType = assetType_;
        assetType = assetType.trimmed();
        assert(!assetType.isEmpty() && "Must specify an asset type for asset factory!");
    }

    virtual QString Type() const { return assetType; }

    virtual AssetPtr CreateEmptyAsset(AssetAPI *, const char *) { return AssetPtr(); }

private:
    QString assetType;
};

#endif
