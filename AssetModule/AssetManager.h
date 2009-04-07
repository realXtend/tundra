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
        AssetManager(Foundation::Framework* framework, OpenSimProtocol::OpenSimProtocolModule* net_interface);
        //! destructor
        virtual ~AssetManager();
        
        //! get asset
        /*! \param asset_id asset UUID
            \param asset_type asset type
            if asset not in cache, will return empty pointer and queue the asset request
         */
        virtual Foundation::AssetPtr GetAsset(const std::string& asset_id, Core::asset_type_t asset_type);

        //! performs time-based update
        /*! uses time to handle timeouts
         */
        void Update(Core::f64 frametime);

        //! requests asset
        /*! \param asset_id asset UUID
            \param asset_type asset type
         */
        void RequestAsset(const RexTypes::RexUUID& asset_id, Core::uint asset_type);
        
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
        //! tries to get asset from disk-based cache
        /*! \param asset_id asset UUID
         */
        void GetFromCache(const RexTypes::RexUUID& asset_id);
        
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
        OpenSimProtocol::OpenSimProtocolModule *net_interface_;
        
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
        
        //! default asset cache path
        static const char *AssetManager::DEFAULT_ASSET_CACHE_PATH;
        
        //! default asset transfer timeout 
        static const Core::f64 AssetManager::DEFAULT_ASSET_TIMEOUT;
    };
}


#endif