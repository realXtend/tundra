// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_LocalAssetProvider_h
#define incl_Asset_LocalAssetProvider_h

#include "ThreadTaskManager.h"

namespace HttpUtilities
{
    class HttpTaskResult;
}

namespace Asset
{
    //! Local asset provider
    /*! Handles local files as if they were remote assets.
     */
    class LocalAssetProvider : public Foundation::AssetProviderInterface
    {
    public:  
        //! Constructor
        /*! \param framework Framework
            \param base_dir Directory for assets
         */
        LocalAssetProvider(Foundation::Framework* framework, const std::string& asset_dir);
        
        //! Destructor
        virtual ~LocalAssetProvider();
        
        //! Returns name of asset provider
        virtual const std::string& Name();
        
        //! Checks an asset id for validity
        /*! \return true if this asset provider can handle the id
         */
        virtual bool IsValidId(const std::string& asset_id, const std::string& asset_type);
        
        //! Requests an asset for "download"
        /*! \param asset_id Asset UUID
            \param asset_type Asset type
            \param tag Asset request tag, allocated by AssetService
            \return true if asset ID was valid and file could be found (ASSET_READY will be sent in that case)
         */
        virtual bool RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag);
        
        //! Returns whether a certain asset is already being "downloaded". Returns always false.
        virtual bool InProgress(const std::string& asset_id);
        
        //! Queries status of asset "download"
        /*! \param asset_id Asset UUID
            \param size Variable to receive asset size (if known, 0 if unknown)
            \param received Variable to receive amount of bytes received
            \param received_continuous Variable to receive amount of continuous bytes received from the start
            \return false will always be returned (not supported)
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous);
        
        //! Gets incomplete asset
        /*! If transfer not in progress or not enough bytes received, will return empty pointer
            
            \param asset_id Asset UUID
            \param asset_type Asset type
            \param received Minimum continuous bytes received from the start
            \return Null pointer will always be returned (not supported)
         */
        virtual Foundation::AssetPtr GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received);   
        
        //! Returns information about current asset transfers
        virtual Foundation::AssetTransferInfoVector GetTransferInfo() { return Foundation::AssetTransferInfoVector(); }
        
        //! Performs time-based update 
        /*! \param frametime Seconds since last frame
         */
        virtual void Update(f64 frametime);
        
    private:
        //! Asset event category
        event_category_id_t event_category_;
        
        //! Framework
        Foundation::Framework* framework_;
        
        //! Asset base directory
        std::string asset_dir_;
    };
}


#endif
