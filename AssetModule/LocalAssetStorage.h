// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_LocalAssetStorage_h
#define incl_Asset_LocalAssetStorage_h

#include "AssetModuleApi.h"
#include "IAssetStorage.h"

class QFileSystemWatcher;

namespace Asset
{

class ASSET_MODULE_API LocalAssetStorage : public IAssetStorage
{
    Q_OBJECT

public:
    LocalAssetStorage();
    ~LocalAssetStorage();

    /// Specifies the absolute path of the storage.
    std::string directory;

    /// Specifies a human-readable name for this storage.
    std::string name;

    /// If true, all subdirectories of the storage directory are automatically looked in when loading an asset.
    bool recursive;

    std::string GetFullPathForAsset(const std::string &assetname, bool recursive);

    /// Starts listening on the local directory this asset storage points to.
    void SetupWatcher();

    /// Stops and deallocates the directory change listener.
    void RemoveWatcher();

    QFileSystemWatcher *changeWatcher;

    QString Name() const { return name.c_str(); }

    QString BaseURL() const { return "file://"; }

private:

    void operator=(const LocalAssetStorage &);
    LocalAssetStorage(const LocalAssetStorage &);
};

}

#endif
