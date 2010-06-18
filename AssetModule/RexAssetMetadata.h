// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_RexAssetMetadata_h
#define incl_Asset_RexAssetMetadata_h

#include "AssetInterface.h"

namespace Asset
{
    //! Asset metadata for reX assets.
    class RexAssetMetadata : public Foundation::AssetMetadataInterface
    {
    public:
        RexAssetMetadata();

        //! AssetMetadataInterface override
        virtual const std::string& GetId() const;

        //! AssetMetadataInterface override
        virtual const std::string& GetName() const;

        //! AssetMetadataInterface override
        virtual const std::string& GetDescription() const;

        //! AssetMetadataInterface override
        virtual const time_t  GetCreationDate() const;

        //! AssetMetadataInterface override
        virtual std::string GetCreationDateString() const;

        //! AssetMetadataInterface override
        virtual const std::string& GetContentType() const;

        //! AssetMetadataInterface override
        virtual std::string GetHashSHA1() const; 

        //! AssetMetadataInterface override
        virtual bool IsTemporary() const;

        //! AssetMetadataInterface override
        virtual const Foundation::MethodMap& GetMethods() const;

        //! AssetMetadataInterface override
        virtual void DesesrializeFromJSON(std::string data);

    private:
        //! Asset id eg. uuid
        std::string id_;

        //! human readable asset name
        std::string name_;

        //! human readable description
        std::string description_;

        //! unix time stamp
        time_t creation_date_;

        //! content type 
        std::string content_type_;

        //! sha1 hash
        u8 hash_sha1_[20];

        //! true if asset is temporary
        bool temporary_;

        //! methods as <method, url> map 
        Foundation::MethodMap methods_;

        //! asset type
        std::string asset_type_;
    };
}

#endif
