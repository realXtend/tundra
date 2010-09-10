// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_XMLRPCAssetProvider_h
#define incl_Asset_XMLRPCAssetProvider_h

#include "ThreadTaskManager.h"

namespace HttpUtilities
{
    class HttpTaskResult;
}

namespace Asset
{
    //! XMLRPC asset provider
    /*! Handles legacy avatar storage XMLRPC asset transfers.
        Created by AssetModule.
     */
    class XMLRPCAssetProvider : public Foundation::AssetProviderInterface
    {
    public:  
        //! Constructor
        /*! \param framework Framework
         */     
        XMLRPCAssetProvider(Foundation::Framework* framework);

        //! Destructor
        virtual ~XMLRPCAssetProvider();
        
        //! Returns name of asset provider
        virtual const std::string& Name();
        
        //! Checks an asset id for validity
        /*! \return true if this asset provider can handle the id
         */
        virtual bool IsValidId(const std::string& asset_id, const std::string& asset_type);
        
        //! Requests an asset for download
        /*! \param asset_id Asset UUID
            \param asset_type Asset type
            \param tag Asset request tag, allocated by AssetService
            \return true if asset ID was valid and download could be queued, false if not 
         */
        virtual bool RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag);
        
        //! Returns whether a certain asset is already being downloaded
        virtual bool InProgress(const std::string& asset_id);
        
        //! Queries status of asset download
        /*! \param asset_id Asset UUID
            \param size Variable to receive asset size (if known, 0 if unknown)
            \param received Variable to receive amount of bytes received
            \param received_continuous Variable to receive amount of continuous bytes received from the start
            \return true If transfer in progress, and variables have been filled, false if transfer not found
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous);       
        
        //! Gets incomplete asset
        /*! If transfer not in progress or not enough bytes received, will return empty pointer
            
            \param asset_id Asset UUID
            \param asset_type Asset type
            \param received Minimum continuous bytes received from the start
            \return Pointer to asset
         */
        virtual Foundation::AssetPtr GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received);   
        
        //! Returns information about current asset transfers
        virtual Foundation::AssetTransferInfoVector GetTransferInfo() { return Foundation::AssetTransferInfoVector(); }
        
        //! Performs time-based update 
        /*! \param frametime Seconds since last frame
         */
        virtual void Update(f64 frametime);
        
    private:
        //! Asset request. Used internally by XMLRPCAssetProvider
        struct AssetRequest
        {
            //! Asset ID
            std::string asset_id_;
            //! Asset type
            std::string asset_type_;
            //! Associated request tags
            RequestTagVector tags_;
            //! Http request tag
            request_tag_t http_request_tag_;
        };
        
        //! Process a http result
        void ProcessHttpResult(HttpUtilities::HttpTaskResult* result);
        
        //! Decode an asset from result data that matches a request
        void DecodeAsset(HttpUtilities::HttpTaskResult* result, const AssetRequest& request);
        
        //! Send notification of asset transfer cancel (fail)
        void SendCanceled(const AssetRequest& request, const std::string& error_message);
        
        //! Current asset transfer timeout
        f64 asset_timeout_;
        
        //! Default asset transfer timeout 
        static const float DEFAULT_ASSET_TIMEOUT;
        
        //! Asset event category
        event_category_id_t event_category_;
        
        //! Framework
        Foundation::Framework* framework_;
        
        //! Current asset requests
        typedef std::map<std::string, AssetRequest> AssetRequestMap;
        AssetRequestMap requests_;
        
        //! Thread task manager for XMLRPC downloads (one task per host)
        Foundation::ThreadTaskManager manager_;
    };
}


#endif
