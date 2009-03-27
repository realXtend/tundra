// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetManager_h
#define incl_Asset_AssetManager_h

#include "AssetServiceInterface.h"
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
        class AssetTransfer
        {
        public:
            AssetTransfer();
            ~AssetTransfer();
            
            void ReceiveData(Core::uint packet_index, const Core::u8* data, Core::uint size);
            void AssembleData(Core::u8* buffer) const;
            
            void SetAssetId(const RexTypes::RexUUID& asset_id) { asset_id_ = asset_id; }
            void SetAssetType(Core::uint asset_type) { asset_type_ = asset_type; }
            void SetSize(Core::uint size) { size_ = size; }
            
            const RexTypes::RexUUID& GetAssetId() const { return asset_id_; }
            Core::uint GetAssetType() const { return asset_type_; }
            Core::uint GetSize() const { return size_; }
            Core::uint GetReceived() const { return received_; }
            Core::uint GetReceivedContinuous() const;
            bool Ready() const;
            
        private:
            typedef std::map<Core::uint, std::vector<Core::u8> > DataPacketMap;
            
            RexTypes::RexUUID asset_id_;
            Core::uint asset_type_;
            Core::uint size_;
            Core::uint received_;
            DataPacketMap data_packets_;
        };
        
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
    };
}


#endif