// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_LocalAssetStorage_h
#define incl_Asset_LocalAssetStorage_h

#include "AssetModuleApi.h"
#include "IAssetStorage.h"

class QFileSystemWatcher;
class AssetAPI;

namespace Asset
{

class ASSET_MODULE_API LocalAssetStorage : public IAssetStorage
{
Q_OBJECT

public:
    LocalAssetStorage();
    ~LocalAssetStorage();

    /// Specifies the absolute path of the storage.
    QString directory;

    /// Specifies a human-readable name for this storage.
    QString name;

    /// If true, all subdirectories of the storage directory are automatically looked in when loading an asset.
    bool recursive;

    /// Starts listening on the local directory this asset storage points to.
    void SetupWatcher();

    /// Stops and deallocates the directory change listener.
    void RemoveWatcher();

    /// \todo Will be replaced with AssetStorage's GetAllAssetsRefs / GetAllAssets functionality
    void LoadAllAssetsOfType(AssetAPI *assetAPI, const QString &suffix, const QString &assetType);

public slots:
    bool Writable() const { return true; }

    /// Returns the full local filesystem path name of the given asset in this storage, if it exists.
    /// Example: GetFullPathForAsset("my.mesh", true) might return "C:\Projects\Tundra\bin\data\assets".
    /// If the file does not exist, returns "".
    QString GetFullPathForAsset(const QString &assetname, bool recursive);

    /// Returns the URL that should be used in a scene asset reference attribute to refer to the asset with the given localName.
    /// Example: GetFullAssetURL("my.mesh") might return "local://my.mesh".
    /// \note LocalAssetStorage ignores all subdirectory specifications, so GetFullAssetURL("data/assets/my.mesh") would also return "local://my.mesh".
    QString GetFullAssetURL(const QString &localName);

    /// Returns all assetrefs contained in this asset storage. Does not load the assets
    virtual QStringList GetAllAssetRefs();
    
//    QFileSystemWatcher *changeWatcher;

    QString Name() const { return name; }

    QString BaseURL() const { return "local://"; }

    QString ToString() const { return Name() + " (" + directory + ")"; }

private:
    void operator=(const LocalAssetStorage &);
    LocalAssetStorage(const LocalAssetStorage &);
};

}

#endif
