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
    std::string directory;
    std::string name;
    bool recursive;

    std::string GetFullPathForAsset(const std::string &assetname, bool recursive);
};

}

#endif
