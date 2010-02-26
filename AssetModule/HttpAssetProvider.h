// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_HttpAssetProvider_h
#define incl_Asset_HttpAssetProvider_h

#include "RexAsset.h"
#include "HttpAssetTransfer.h"
#include "AssetProviderInterface.h"

namespace Asset
{    
    //! Http asset provider
    /*! Provides asset downloading with http protocol.
        Created by AssetModule.
     */
    class HttpAssetProvider : public Foundation::AssetProviderInterface
    {
    public:  
        //! Constructor
        /*! \param framework Framework

            throws Exception in case of an error.
         */     
        HttpAssetProvider(Foundation::Framework* framework);

        //! Destructor
        virtual ~HttpAssetProvider();
        
        //! Returns name of asset provider
        virtual const std::string& Name();
        
        //! Checks an asset id for validity
        /*! \return true if this asset provider can handle the id
         */
        virtual bool IsValidId(const std::string& asset_url);
        
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
                
        //! Handles incoming network packet event
        /*! \return true if event handled
         */
        bool HandleNetworkEvent(Foundation::EventDataInterface* data);
        
    private:
        
        std::string SerializeToJSON(Foundation::AssetPtr asset) const;
        
        //! Stores completed asset to asset service's cache
        void StoreAsset(HttpAssetTransfer& transfer);
        
        //! Gets asset transfer if it's in progress
        /*! \param asset_id Asset ID
            \return Pointer to transfer, or 0 if no transfer
         */
        HttpAssetTransfer* GetTransfer(const std::string& asset_url);
        
        //! Sends progress event of asset transfer
        /*! \param transfer Asset transfer
         */
        void SendAssetProgress(HttpAssetTransfer& transfer);

        //! Sends asset transfer canceled event
        /*! \param transfer Asset transfer
         */
        void SendAssetCanceled(HttpAssetTransfer& transfer);
           
        typedef boost::shared_ptr<HttpAssetTransfer> HttpAssetTransferPtr;
        typedef std::map<std::string, HttpAssetTransferPtr> HttpAssetTransferMap;
        typedef std::pair<std::string, HttpAssetTransfer> HttpAssetTransferEntry;
                
        //! Ongoing Http asset transfers, keyed by transfer url
        HttpAssetTransferMap asset_transfers_;
        
        //! Asset event category
        event_category_id_t event_category_;
        
        //! Current asset transfer timeout
        f64 asset_timeout_;
        
        //! Default asset transfer timeout 
        static const Real DEFAULT_ASSET_TIMEOUT;
        
        //! Framework
        Foundation::Framework* framework_;    

    };
} // end of namespace: Asset

#endif // incl_Asset_HttpAssetProvider_h
