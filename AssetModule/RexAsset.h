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
        //! default constructor
        RexAsset() {};
        
        //! destructor
        virtual ~RexAsset() {};
        
        //! returns asset ID, which is UUID in text form
        virtual const std::string GetId();
        
        //! returns asset type
        virtual Core::asset_type_t GetType() { return asset_type_; }
        
        //! returns asset type in text form
        virtual const std::string& GetTypeName();
        
        //! returns asset data size
        virtual Core::uint GetSize() { return data_.size(); }
        
        //! returns asset data
        virtual const Core::u8* GetData() { return &data_[0]; }
        
        //! asset id
        RexTypes::RexUUID asset_id_;
        //! asset type
        Core::asset_type_t asset_type_;
        //! asset data
        std::vector<Core::u8> data_;
    };
}

#endif