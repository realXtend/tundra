// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetCache_h
#define incl_Asset_AssetCache_h

#include "Poco/MD5Engine.h"

namespace Asset
{
    //! Stores assets to memory and/or disk based cache. Created and used by AssetManager.
	//! \todo Add support for urls as asset id
    class AssetCache
    {
    public:
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

    private:
        //! Check contents of disk cache
        void CheckDiskCache();

		//! Calculates hash from given asset id
		//! Used for file name generation
		std::string GetHash(const std::string &asset_id);

        
        //! Asset memory cache
        typedef std::map<std::string, Foundation::AssetPtr> AssetMap;
        AssetMap assets_;

        //! Current disk asset cache path
        std::string cache_path_;

        //! Default disk asset cache path
        static const char *DEFAULT_ASSET_CACHE_PATH;

        //! Assets known to be in disk cache
		//! Values are hash values from asset id's
        std::set<std::string> disk_cache_contents_;
        
        //! Framework
        Foundation::Framework* framework_;

		//! MD5 Engine
		Poco::MD5Engine md5_engine_;
    };
}

#endif