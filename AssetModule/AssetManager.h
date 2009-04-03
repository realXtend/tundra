// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetManager_h
#define incl_Asset_AssetManager_h

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
    //! ReX asset
    class Asset
    {
    public:
        Core::uint asset_type_;
        std::vector<Core::u8> data_;
    };
    
    //! Asset manager. Initiates transfers based on asset requests and responds to received data.
    //! \todo cache to disk, handle timeouts, comment code
    class AssetManager : public Foundation::AssetServiceInterface
    {
    public:
        AssetManager(Foundation::Framework* framework, OpenSimProtocol::OpenSimProtocolModule* net_interface);
        virtual ~AssetManager();
        
        virtual Foundation::AssetPtr GetAsset(const std::string& asset_id);

        void RequestAsset(const RexTypes::RexUUID& asset_id, Core::uint asset_type);
        
        void HandleTextureHeader(NetInMessage* msg);
        void HandleTextureData(NetInMessage* msg);
        void HandleTextureCancel(NetInMessage* msg);
        
        void HandleAssetHeader(NetInMessage* msg);
        void HandleAssetData(NetInMessage* msg);
        void HandleAssetCancel(NetInMessage* msg);
        
    private:
        void GetFromCache(const RexTypes::RexUUID& asset_id);
        void RequestTexture(const RexTypes::RexUUID& asset_id);
        void RequestOtherAsset(const RexTypes::RexUUID& asset_id, Core::uint asset_type);
        void StoreAsset(AssetTransfer& transfer);
    
        //! network interface
        OpenSimProtocol::OpenSimProtocolModule *net_interface_;
        
        //! framework we belong to
        Foundation::Framework* framework_;
        
        typedef std::map<RexTypes::RexUUID, AssetTransfer> AssetTransferMap;
        
        typedef std::map<RexTypes::RexUUID, Asset> AssetMap;
        
        //! completely received assets
        AssetMap assets_;
        
        //! ongoing UDP asset transfers, keyed by transfer id
        AssetTransferMap asset_transfers_;
        
        //! ongoing UDP texture transfers, keyed by texture asset id
        AssetTransferMap texture_transfers_;
        
        //! current asset cache path
        std::string cache_path_;
        
        //! default asset cache path
        static const char *AssetManager::DEFAULT_ASSET_CACHE_PATH;
    };
}


#endif