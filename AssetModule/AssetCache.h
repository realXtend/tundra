// For conditions of distribution and use, see copyright notice in license.txt

namespace Asset
{
    //! Stores assets to memory and/or disk based cache. Used by AssetManager.
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
        /*! \param transfer Finished asset transfer
         */
        void StoreAsset(AssetTransfer& transfer);

        //! Stores asset to cache. Posts ASSET_READY event when done.
        /*! \param asset Asset
         */
        void StoreAsset(Foundation::AssetPtr asset);

    private:
        typedef std::map<std::string, Foundation::AssetPtr> AssetMap;
        
        //! Completely received assets (memory cache)
        AssetMap assets_;

        //! Current disk asset cache path
        std::string cache_path_;

        //! Default disk asset cache path
        static const char *DEFAULT_ASSET_CACHE_PATH;

        //! Framework
        Foundation::Framework* framework_;
    };
}
