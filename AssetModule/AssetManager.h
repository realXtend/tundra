// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetManager_h
#define incl_Asset_AssetManager_h

#include "AssetInterface.h"
#include "AssetServiceInterface.h"
#include "AssetProviderInterface.h"
#include "AssetTransfer.h"
#include "RexUUID.h"

namespace Foundation
{
    class Framework;
}

namespace Asset
{
    class AssetCache;

    //! Asset manager. Implements the AssetServiceInterface.
    /*! \ingroup AssetModuleClient
        Initiates transfers based on asset requests and responds to received data.
        See \ref AssetModule for details on how to use the asset service.
     */
    class AssetManager : public Foundation::AssetServiceInterface
    {
    public:
        //! Constructor
        AssetManager(Foundation::Framework* framework);
        //! Destructor
        virtual ~AssetManager();
        
        //! Gets asset
        /*! \param asset_id Asset ID, UUID for legacy UDP assets
            \param asset_type Asset type
            \return Pointer to asset, NULL if not found
         */
        virtual Foundation::AssetPtr GetAsset(const std::string& asset_id, Core::asset_type_t asset_type);

        //! Gets incomplete asset
        /*! Note: a new incomplete asset object (with copy of the data) will be created for each call. Please
            do not store the shared pointer for longer than necessary.
            
            \param asset_id Asset ID, UUID for legacy UDP assets
            \param asset_type Asset type
            \param received Minimum continuous bytes received from the start
            \return Pointer to asset, NULL if not found or not enough bytes
           
         */
        virtual Foundation::AssetPtr GetIncompleteAsset(const std::string& asset_id, Core::asset_type_t asset_type, Core::uint received);
        
        //! Requests an asset download
        /*! If asset already downloaded, does nothing.
            Events will be sent when download progresses, and when asset is ready.

            \param asset_id Asset ID, UUID for legacy UDP assets
            \param asset_type Asset type
            \return true if 
         */
        virtual bool RequestAsset(const std::string& asset_id, Core::asset_type_t asset_type);

        //! Queries status of asset download
        /*! If asset has been already fully received, size, received & received_continuous will be the same
        
            \param asset_id Asset ID, UUID for legacy UDP assets
            \param size Variable to receive asset size (if known, 0 if unknown)
            \param received Variable to receive amount of bytes received
            \param received_continuous Variable to receive amount of continuous bytes received from the start
            \return true if asset was found either in cache or as a transfer in progress, and variables have been filled, false if not found
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, Core::uint& size, Core::uint& received, Core::uint& received_continuous);
        
        //! Registers an asset provider
        /*! \param asset_provider Provider to register
            \return true if successfully registered
         */
        virtual bool RegisterAssetProvider(Foundation::AssetProviderPtr asset_provider);
        
        //! Unregisters an asset provider
        /*! \param asset_provider Provider to unregister
            \return true if successfully unregistered
         */       
        virtual bool UnregisterAssetProvider(Foundation::AssetProviderPtr asset_provider);
                
        //! Stores an asset to the asset cache    
        /*! \param asset Asset to store
         */
        virtual void StoreAsset(Foundation::AssetPtr asset);
        
        //! Performs time-based update
        /*! Calls update function of all registered asset providers
            \param frametime Seconds since last frame
         */
        void Update(Core::f64 frametime);            
        
    private:      
       //! Gets asset from cache
       /*! \param asset_id Asset ID
        */
        Foundation::AssetPtr GetFromCache(const std::string& asset_id);
          
        //! Framework we belong to
        Foundation::Framework* framework_;
                                
        //! Asset event category
        Core::event_category_id_t event_category_;
                
        //! Asset cache
        typedef boost::shared_ptr<AssetCache> AssetCachePtr;
        AssetCachePtr cache_;
        
        //! Asset providers
        typedef std::vector<Foundation::AssetProviderPtr> AssetProviderVector;
        AssetProviderVector providers_;
    };
}


#endif
