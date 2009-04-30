// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_RexAsset_h
#define incl_Asset_RexAsset_h

#include "AssetInterface.h"
#include "RexUUID.h"

namespace Asset
{
    class AssetTransfer;
    
    //! ReX asset. Implements the AssetInterface.
    /*! \ingroup AssetModuleClient
     */ 
    class RexAsset : public Foundation::AssetInterface
    {
    public:
        typedef std::vector<Core::u8> AssetDataVector;
        
        //! constructor
        RexAsset(const std::string& asset_id, Core::asset_type_t asset_type);
        
        //! destructor
        virtual ~RexAsset() {};
        
        //! returns asset ID
        virtual const std::string GetId() { return asset_id_; }
        
        //! returns asset type
        virtual Core::asset_type_t GetType() { return asset_type_; }
        
        //! returns asset type in text form
        virtual const std::string& GetTypeName();
        
        //! returns asset data size
        virtual Core::uint GetSize() { return data_.size(); }
        
        //! returns asset data
        virtual const Core::u8* GetData() { return &data_[0]; }
        
        //! returns asset data vector, non-const. For internal use
        AssetDataVector& GetDataInternal() { return data_; }
        
    private:
        //! asset id
        std::string asset_id_;
        //! asset type
        Core::asset_type_t asset_type_;
        //! asset data
        AssetDataVector data_;
    };
}

#endif