// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AssetAPI.h"
#include "IAssetBundle.h"
#include "ZipWorker.h"

#include <boost/shared_ptr.hpp>

struct zzip_dir;

class ZipAssetBundle : public IAssetBundle
{
Q_OBJECT

public:
    ZipAssetBundle(AssetAPI *owner, const QString &type, const QString &name);
    ~ZipAssetBundle();

    /// IAsset override.
    virtual bool IsLoaded() const;

    /// IAsset override.
    virtual void DoUnload();

    /// IAssetBundle override.
    virtual bool RequiresDiskSource() { return true; }

    /// IAssetBundle override.
    /** Our current zzip implementation seems to require disk source for processing.
        So we fail DeserializeFromData and try our best here to.
        This function unpacks all the archive content to asset cache to its own bundle folder
        and provides the sub asset data via GetSubAssetData and GetSubAssetDiskSource. */
    virtual bool DeserializeFromDiskSource();

    /// IAssetBundle override.
    /** @todo If we must support this in memory method with zzip 
        we could store the data to disk and open it. Be sure to change RequiresDiskSource to false. */
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes);

    /// IAssetBundle override.
    virtual std::vector<u8> GetSubAssetData(const QString &subAssetName);

    /// IAssetBundle override.
    virtual QString GetSubAssetDiskSource(const QString &subAssetName);
    
private slots:
    /// Returns full asset reference for a sub asset.
    QString GetFullAssetReference(const QString &subAssetName);
    
    /// Handler for asynch loading completion.
    void OnAsynchLoadCompleted(bool successful);
    
private:
    void Close();
    void CloseWorker();

    zzip_dir *archive_;
    ZipFileList files_;
    ZipWorker *worker_;
};

typedef boost::shared_ptr<ZipAssetBundle> ArchiveAssetPtr;
