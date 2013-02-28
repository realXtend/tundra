// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AssetAPI.h"
#include "IAssetBundle.h"
#include "ZipWorker.h"

struct zzip_dir;

/// Provides zip packed asset bundle support.
class ZipAssetBundle : public IAssetBundle
{
    Q_OBJECT

public:
    ZipAssetBundle(AssetAPI *owner, const QString &type, const QString &name);
    ~ZipAssetBundle();

    /// IAssetBundle override.
    virtual bool IsLoaded() const;

    /// IAssetBundle override.
    virtual void DoUnload();

    /// IAssetBundle override.
    /** Our current zziplib implementation requires disk source for processing. */
    virtual bool RequiresDiskSource() { return true; }

    /// IAssetBundle override.
    /** Our current zziplib implementation requires disk source for processing.
        So we fail DeserializeFromData and try our best here to.
        This function unpacks the archive content to asset cache to normal cache files
        and provides the sub asset data via GetSubAssetData and GetSubAssetDiskSource. */
    virtual bool DeserializeFromDiskSource();

    /// IAssetBundle override.
    /** @todo If we must support this in memory method with zzip 
        we could store the data to disk and open it. Be sure to change RequiresDiskSource to false.
        @return Currently not applicable, so false always. */
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
    /// Closes zip file.
    void Close();
    
    /// Zziplib ptr to the zip file.
    zzip_dir *archive_;
    
    /// Zip sub assets.
    ZipFileList files_;
};

typedef shared_ptr<ZipAssetBundle> ArchiveAssetPtr;
