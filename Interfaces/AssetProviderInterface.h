// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetProviderInterface_h
#define incl_Interfaces_AssetProviderInterface_h

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    class NetInMessage;
}

namespace Foundation
{
    class AssetProviderInterface;
    typedef boost::shared_ptr<AssetProviderInterface> AssetProviderPtr;
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
 
    //! Asset transfer info
    struct AssetTransferInfo
    {
        std::string id_;
        std::string type_;
        std::string provider_;
        uint size_;
        uint received_;
        uint received_continuous_;
    };
   
    typedef std::vector<AssetTransferInfo> AssetTransferInfoVector;
        
    /*! Asset provider. Can be registered to the AssetService to add possibility of downloading assets by different
        means (legacy UDP, http etc.)
        
        Asset providers receive asset download requests through the RequestAsset() function. It should
        return true if the asset id was of such format that the request can be handled (for example
        a valid UUID for legacy UDP assets), false if it could not be handled.
        
        When an asset download is ready, the asset provider has the responsibility of storing it to the asset cache
        by calling StoreAsset() function in the AssetService.
        
        Additionally, the asset provider can post events of the progress of an asset download.
     */
    class AssetProviderInterface
    {
    public:
        AssetProviderInterface() {}
        virtual ~AssetProviderInterface() {}   

        //! Returns name of asset provider for identification purposes
        virtual const std::string& Name() = 0;

        //! Checks an asset id for validity
        /*! \return true if this asset provider can handle the id
         */
        virtual bool IsValidId(const std::string& asset_id, const std::string& asset_type) = 0;

        //! Requests an asset for download
        /*! Note: implementation should not queue multiple transfers if for some reason RequestAsset gets called
            multiple times for the same asset. However, they should store all the tags associated with the same
            transfer, and then send an ASSET_READY event for each tag (if multiple), when that transfer finishes. 

            \param asset_id Asset ID
            \param asset_type Asset type
            \param tag Asset request tag, allocated by AssetService. To be sent back along with ASSET_READY event
            \return true if asset ID was valid and download could be queued, false if not 
         */
        virtual bool RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag) = 0;

        //! Returns whether a certain asset is already being downloaded
        /*! \param asset_id Asset ID
         */           
        virtual bool InProgress(const std::string& asset_id) = 0;

        //! Queries status of asset download
        /*! If asset provider receives data only in ordered manner (http requests etc.) received & received_continuous 
            should be the same.

            \param asset_id Asset ID
            \param size Variable to receive asset size (if known, 0 if unknown)
            \param received Variable to receive amount of bytes received
            \param received_continuous Variable to receive amount of continuous bytes received from the start
            \return true If transfer in progress, and variables have been filled, false if transfer not found
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous) = 0;       

        //! Gets incomplete asset data from a transfer in progress
        /*! If transfer not in progress or not enough bytes received, should return empty pointer.
            
            \param asset_id Asset ID
            \param asset_type Asset type
            \param received Minimum continuous bytes received from the start
            \return Pointer to asset
         */
        virtual AssetPtr GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received) = 0;   

        //! Returns information about current asset transfers
        virtual AssetTransferInfoVector GetTransferInfo() = 0;

        //! Sets current protocolmodule
        virtual void SetCurrentProtocolModule(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> protocolModule) {};

        //! Performs time-based update of asset provider, to for example handle timeouts
        /*! The asset service will call this periodically for all registered asset providers, so
            it does not need to be called manually.
            \param frametime Seconds since last frame
         */
        virtual void Update(f64 frametime) {};
    };
}

#endif
