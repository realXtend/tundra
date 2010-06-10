// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetCache_h
#define incl_Asset_AssetCache_h

class QCryptographicHash;

#include "Foundation.h"
#include "AssetInterface.h"

#include <QObject>
#include <QDir>

namespace Asset
{
    //! Stores assets to memory and/or disk based cache. Created and used by AssetManager.
    class AssetCache : public QObject
    {

    Q_OBJECT

    public:
        typedef std::map<std::string, Foundation::AssetPtr> AssetMap;

        //! Constructor
        /*! \param framework Framework
         */ 
        AssetCache(Foundation::Framework* framework);

        //! Destructor
        ~AssetCache();

        //! Tries to get asset from cache, memory first, then disk
        /*! \param asset_id Asset ID
            \param check_memory Whether to check memory cache
            \param check_disk Whether to check disk cache
            \return Pointer to asset if found, or null if not
         */
        Foundation::AssetPtr GetAsset(const std::string& asset_id, bool check_memory = true, bool check_disk = true);

        //! Stores asset to cache. Posts ASSET_READY event when done.
        /*! \param asset Asset
         */
        void StoreAsset(Foundation::AssetPtr asset);

        //! Deletes the memory asset
        bool DeleteAsset(Foundation::AssetPtr asset);

        //! Returns all assets
        const AssetMap& GetAssets() const { return assets_; }
        
        //! Update. Adds age to assets, removes oldest if cache size too big
        void Update(f64 frametime);

    private slots:
        void ClearDiskCache();
        void CacheConfigChanged(int new_disk_max_size);
        void CheckDiskCacheSize(bool make_extra_space = true);

    private:
        //! Read config and init QDir to working directory
        void ReadConfig();

        //! Check contents of a disk cache path
        /*! \param path Disk cache path
         */
        void CheckDiskCache(const std::string& path);

        //! Calculates hash from given asset id
        //! Used for file name generation
        std::string GetHash(const std::string &asset_id);

        //! Asset memory cache
        AssetMap assets_;

        //! Current disk asset cache path
        std::string cache_path_;
        
        //! Maximum memory cache size
        uint memory_cache_size_;
        
        //! Update time accumulator
        f64 update_time_;

        //! Assets known to be in disk cache
        //! Values are hash values from asset id's
        std::set<std::string> disk_cache_contents_;

        //! Framework
        Foundation::Framework* framework_;

        //! MD5 Engine
        QCryptographicHash *md5_engine_;

        QDir cache_dir_;
        int disk_cache_max_size_;
        bool disk_changes_after_last_check_;
    };
}

#endif