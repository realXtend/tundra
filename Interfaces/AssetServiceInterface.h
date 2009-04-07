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

        //! get asset
        /*! \param asset_id asset UUID
            \param asset_type asset type
            if asset not in cache, will return empty pointer and queue the asset request
         */
        virtual AssetPtr GetAsset(const std::string& asset_id, Core::asset_type_t asset_type) = 0;
    };
}

#endif
