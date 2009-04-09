// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetServiceInterface_h
#define incl_Interfaces_AssetServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
    
    class AssetServiceInterface : public ServiceInterface
    {
    public:
        AssetServiceInterface() {}
        virtual ~AssetServiceInterface() {}

        //! gets asset
        /*! if asset not in cache, will return empty pointer and queue the asset request.
        
            \param asset_id asset ID, UUID for legacy UDP assets
            \param asset_type asset type
            \return pointer to asset
            
         */
        virtual AssetPtr GetAsset(const std::string& asset_id, Core::asset_type_t asset_type) = 0;
        
        //! gets incomplete asset
        /*! if asset not yet requested, will request it and return empty pointer
            if not enough bytes received, will return empty pointer
            
            \param asset_id asset ID, UUID for legacy UDP assets
            \param asset_type asset type
            \param received minimum continuous bytes received from the start
            \return pointer to asset
         */
        virtual AssetPtr GetIncompleteAsset(const std::string& asset_id, Core::asset_type_t asset_type, Core::uint received) = 0;
        
        //! queries status of asset download
        /*! if asset has been already fully received, size, received & received_continuous will be the same
        
            \param asset_id asset ID, UUID for legacy UDP assets
            \param size variable to receive asset size (if known, 0 if unknown)
            \param received variable to receive amount of bytes received
            \param received_continuous variable to receive amount of continuous bytes received from the start
            \return true if asset was found either in cache or as a transfer in progress, and variables have been filled, false if not found
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, Core::uint& size, Core::uint& received, Core::uint& received_continuous) = 0;
    };
}

#endif
