// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetTransfer_h
#define incl_Asset_AssetTransfer_h

#include "RexUUID.h"

namespace Asset
{
    //! An asset transfer in progress
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
}

#endif