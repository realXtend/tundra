// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_LocalAssetStorage_h
#define incl_Asset_LocalAssetStorage_h

#include "AssetModuleApi.h"
#include "IAssetStorage.h"

namespace Asset
{

class ASSET_MODULE_API LocalAssetStorage : public IAssetStorage
{
    Q_OBJECT

public:
    /// Specifies the absolute path of the storage.
    std::string directory;

    /// Specifies a human-readable name for this storage.
    std::string name;

    /// If true, all subdirectories of the storage directory are automatically looked in when loading an asset.
    bool recursive;

    std::string GetFullPathForAsset(const std::string &assetname, bool recursive);
};

}

#endif
