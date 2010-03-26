// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_RexAssetMetadata_h
#define incl_Asset_RexAssetMetadata_h

#include "AssetInterface.h"
#include "RexUUID.h"

namespace Asset
{
	//! Implementation of asset metadata 
	//! 
	//!
	class RexAssetMetadata : public Foundation::AssetMetadataInterface
	{
	public:
		RexAssetMetadata();

		//! Returns asset id
		virtual const std::string& GetId() const;

		//! Returns asset name
		virtual const std::string& GetName() const;

		//! Returns description of asset
		virtual const std::string& GetDescription() const;

		//! Returns creation date of asset
		//!   format: unix time stamp is now used
		//!   \todo better date/time type should be used
		virtual const int GetCreationDate() const;

		//! Returns content type
		virtual const std::string& GetContentType() const;

		//! Return SHA1 hash of asset data
		//! Format: Hexadecimal (40 char length string)
		//! /note NOT IMPLEMENTED 
		virtual std::string GetHashSHA1() const; 

		//! Return true id asset is temporary
		virtual bool IsTemporary() const;

		//! Return methods as <method, url> map 
		virtual const Foundation::MethodMap& GetMethods() const;

		//! Parse json encoded metadata 
		virtual void DesesrializeFromJSON(std::string data);

	private:
		//! Asset id eg. uuid
		std::string id_;

		//! human readable asset name
		std::string name_;

		//! human readable description
		std::string description_;

		//! unix time stamp
		int creation_date_;

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

} // end of namespace: Asset

#endif // incl_Asset_RexAssetMetadata_h
