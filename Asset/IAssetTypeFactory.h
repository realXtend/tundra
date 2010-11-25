// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetFactory_h
#define incl_Asset_AssetFactory_h

#include <QString>
#include "AssetFwd.h"

class IAssetTypeFactory
{
public:
    virtual ~IAssetTypeFactory() {}

    /// Returns the type of assets this asset type factory can create.
    virtual QString Type() const = 0;

    /// Creates a new asset of the given type that is initialized to the "empty" asset of this type.
    /// @param name The name to give for this asset.
    virtual IAsset *CreateEmptyAsset(const char *name) = 0;
};

#endif
