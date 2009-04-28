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
    //! Asset manager. Implements the AssetServiceInterface.
    /*! \ingroup AssetModuleClient
        Initiates transfers based on asset requests and responds to received data.
        \todo split UDP downloader and assetcache(s) into separate classes
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
         */
        virtual void RequestAsset(const std::string& asset_id, Core::asset_type_t asset_type);

        //! Queries status of asset download
        /*! If asset has been already fully received, size, received & received_continuous will be the same
        
            \param asset_id Asset ID, UUID for legacy UDP assets
            \param size Variable to receive asset size (if known, 0 if unknown)
            \param received Variable to receive amount of bytes received
            \param received_continuous Variable to receive amount of continuous bytes received from the start
            \return true if asset was found either in cache or as a transfer in progress, and variables have been filled, false if not found
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, Core::uint& size, Core::uint& received, Core::uint& received_continuous);
        
        //! Performs time-based update
        /*! uses time to handle timeouts
         */
        void Update(Core::f64 frametime);

        //! Set asset transfer timeout
        void SetTimeout(Core::f64 timeout) { asset_timeout_ = timeout; }
        
        //! Get asset transfer timeout
        Core::f64 GetTimeout() const { return asset_timeout_; }
        
        //! Handles texture header message
        /*! Called by AssetModule
            \param msg Message
         */
        void HandleTextureHeader(NetInMessage* msg);
        
        //! Handles texture data message
        /*! Called by AssetModule
            \param msg Message
         */
        void HandleTextureData(NetInMessage* msg);
        
        //! Handles texture transfer abort message
        /*! Called by AssetModule
            \param msg Message
         */
        void HandleTextureCancel(NetInMessage* msg);
        
        //! Handles other asset transfer header message
        /*! Called by AssetModule
            \param msg Message
         */
        void HandleAssetHeader(NetInMessage* msg);
        
        //! Handles other asset transfer data message
        /*! Called by AssetModule
            \param msg Message
         */
        void HandleAssetData(NetInMessage* msg);
        
        //! Handles other asset transfer abort message
        /*! Called by AssetModule
            \param msg Message
         */
        void HandleAssetCancel(NetInMessage* msg);
        
    private:
        //! Tries to get asset from cache, memory first, then disk
        /*! \param asset_id Asset UUID
            \return Pointer to asset if found, or null if not
         */
        Foundation::AssetPtr GetFromCache(const RexTypes::RexUUID& asset_id);
       
        //! Gets asset transfer if it's in progress
        /*! \param asset_id Asset UUID
            \return Pointer to transfer, or NULL if no transfer
         */
        AssetTransfer* GetTransfer(const RexTypes::RexUUID& asset_id);
        
        //! Requests a texture from network
        /*! \param asset_id Asset UUID
         */
        void RequestTexture(const RexTypes::RexUUID& asset_id);
        
        //! Requests an other asset from network
        /*! \param asset_id Asset UUID
         */
        void RequestOtherAsset(const RexTypes::RexUUID& asset_id, Core::uint asset_type);
        
        //! Sends progress event of asset transfer
        /*! \param transfer Asset transfer
         */
        void SendAssetProgress(AssetTransfer& transfer);

        //! Sends asset transfer canceled event
        /*! \param transfer Asset transfer
         */
        void SendAssetCanceled(AssetTransfer& transfer);

        //! Stores asset to memory & disk caches
        /*! \param transfer Finished asset transfer
         */
        void StoreAsset(AssetTransfer& transfer);

        //! Framework we belong to
        Foundation::Framework* framework_;
        
        typedef std::map<RexTypes::RexUUID, AssetTransfer> AssetTransferMap;
        
        typedef std::map<RexTypes::RexUUID, Foundation::AssetPtr> AssetMap;
        
        //! Completely received assets (memory cache)
        AssetMap assets_;
        
        //! Ongoing UDP asset transfers, keyed by transfer id
        AssetTransferMap asset_transfers_;
        
        //! Ongoing UDP texture transfers, keyed by texture asset id
        AssetTransferMap texture_transfers_;
        
        //! Current asset cache path
        std::string cache_path_;
        
        //! Current asset transfer timeout
        Core::f64 asset_timeout_;
        
        //! Asset event category
        Core::event_category_id_t event_category_;
        
        //! Default asset cache path
        static const char *DEFAULT_ASSET_CACHE_PATH;
        
        //! Default asset transfer timeout 
        static const Core::Real DEFAULT_ASSET_TIMEOUT;
    };
}


#endif
