// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetManager_h
#define incl_Asset_AssetManager_h

#include "AssetInterface.h"
#include "AssetServiceInterface.h"
#include "AssetTransfer.h"
#include "RexUUID.h"

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class OpenSimProtocolModule;
}

class NetInMessage;

namespace Asset
{
    //! Asset manager. Initiates transfers based on asset requests and responds to received data.
    /*! \todo handle timeouts, define service interface
     */
    class AssetManager : public Foundation::AssetServiceInterface
    {
    public:
        //! constructor
        AssetManager(Foundation::Framework* framework);
        //! destructor
        virtual ~AssetManager();
        
        //! gets asset
        /*! if asset not in cache, will return empty pointer and queue the asset request. An event will
            be sent when the asset has been downloaded.
            
            \param asset_id asset UUID
            \param asset_type asset type
            \return pointer to asset

         */
        virtual Foundation::AssetPtr GetAsset(const std::string& asset_id, Core::asset_type_t asset_type);

        //! gets incomplete asset
        /*! if asset not yet requested, will request it and return empty pointer
            if not enough bytes received, will return empty pointer
            note: a new incomplete asset object (with copy of the data) will be created for each call. Please
            do not store the shared pointer for longer than necessary.
            
            \param asset_id asset UUID
            \param asset_type asset type
            \param received minimum continuous bytes received from the start
            \return pointer to asset
           
         */
        virtual Foundation::AssetPtr GetIncompleteAsset(const std::string& asset_id, Core::asset_type_t asset_type, Core::uint received);
        
        //! queries status of asset download
        /*! if asset has been already fully received, size, received & received_continuous will be the same
        
            \param asset_id asset ID, UUID for legacy UDP assets
            \param size variable to receive asset size (if known, 0 if unknown)
            \param received variable to receive amount of bytes received
            \param received_continuous variable to receive amount of continuous bytes received from the start
            \return true if asset was found either in cache or as a transfer in progress, and variables have been filled, false if not found
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, Core::uint& size, Core::uint& received, Core::uint& received_continuous);
        
        //! performs time-based update
        /*! uses time to handle timeouts
         */
        void Update(Core::f64 frametime);

        //! set asset transfer timeout
        void SetTimeout(Core::f64 timeout) { asset_timeout_ = timeout; }
        
        //! get asset transfer timeout
        Core::f64 GetTimeout() const { return asset_timeout_; }
        
        //! handles texture header message
        /*! called by AssetModule
            \param msg message
         */
        void HandleTextureHeader(NetInMessage* msg);
        
        //! handles texture data message
        /*! called by AssetModule
            \param msg message
         */
        void HandleTextureData(NetInMessage* msg);
        
        //! handles texture transfer abort message
        /*! called by AssetModule
            \param msg message
         */
        void HandleTextureCancel(NetInMessage* msg);
        
        //! handles other asset transfer header message
        /*! called by AssetModule
            \param msg message
         */
        void HandleAssetHeader(NetInMessage* msg);
        
        //! handles other asset transfer data message
        /*! called by AssetModule
            \param msg message
         */
        void HandleAssetData(NetInMessage* msg);
        
        //! handles other asset transfer abort message
        /*! called by AssetModule
            \param msg message
         */
        void HandleAssetCancel(NetInMessage* msg);
        
    private:
        //! tries to get asset from cache, memory first, then disk
        /*! \param asset_id asset UUID
            \return asset if found, or null if not
         */
        Foundation::AssetPtr GetFromCache(const RexTypes::RexUUID& asset_id);
       
        //! gets asset transfer if it's in progress
        /*! \param asset_id asset UUID
            \return pointer to transfer, or NULL if no transfer
         */
        AssetTransfer* GetTransfer(const RexTypes::RexUUID& asset_id);
        
        //! requests a texture from network
        /*! \param asset_id asset UUID
         */
        void RequestTexture(const RexTypes::RexUUID& asset_id);
        
        //! requests an other asset from network
        /*! \param asset_id asset UUID
         */
        void RequestOtherAsset(const RexTypes::RexUUID& asset_id, Core::uint asset_type);
        
        //! stores asset to memory & disk caches
        /*! \param transfer finished asset transfer
         */
        void StoreAsset(AssetTransfer& transfer);
    
        //! network interface
        boost::weak_ptr<OpenSimProtocol::OpenSimProtocolModule> net_interface_;
        
        //! framework we belong to
        Foundation::Framework* framework_;
        
        typedef std::map<RexTypes::RexUUID, AssetTransfer> AssetTransferMap;
        
        typedef std::map<RexTypes::RexUUID, Foundation::AssetPtr> AssetMap;
        
        //! completely received assets (memory cache)
        AssetMap assets_;
        
        //! ongoing UDP asset transfers, keyed by transfer id
        AssetTransferMap asset_transfers_;
        
        //! ongoing UDP texture transfers, keyed by texture asset id
        AssetTransferMap texture_transfers_;
        
        //! current asset cache path
        std::string cache_path_;
        
        //! current asset transfer timeout
        Core::f64 asset_timeout_;
        
        //! asset event category
        Core::event_category_id_t event_category_;
        
        //! default asset cache path
        static const char *DEFAULT_ASSET_CACHE_PATH;
        
        //! default asset transfer timeout 
        static const Core::Real DEFAULT_ASSET_TIMEOUT;
    };
}


#endif
