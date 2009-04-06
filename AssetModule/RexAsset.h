// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_RexAsset_h
#define incl_Asset_RexAsset_h

#include "AssetInterface.h"
#include "RexUUID.h"

namespace Asset
{
    class AssetTransfer;
    
    //! ReX asset
    class RexAsset : public Foundation::AssetInterface
    {
    public:
        RexAsset() {};
        virtual ~RexAsset() {};
        
        virtual const std::string GetId();
        virtual Core::asset_type_t GetType() { return asset_type_; }
        virtual const std::string& GetTypeName();
        virtual Core::uint GetSize() { return data_.size(); }
        virtual const Core::u8* GetData() { return &data_[0]; }
        
        void GetDataFromTransfer(AssetTransfer& transfer);
        
        //! asset id
        RexTypes::RexUUID asset_id_;
        //! asset type
        Core::asset_type_t asset_type_;
        //! asset data
        std::vector<Core::u8> data_;
    };
}

#endif