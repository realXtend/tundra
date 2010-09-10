// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_UDPAssetProvider_h
#define incl_Asset_UDPAssetProvider_h

#include "UDPAssetTransfer.h"
#include "AssetProviderInterface.h"

namespace Asset
{
    //! UDP asset provider
    /*! Handles legacy UDP texture & asset transfers using OpenSimProtocolModule network events.
        Created by AssetModule.
     */
    class UDPAssetProvider : public Foundation::AssetProviderInterface
    {
    public:
        //! Constructor
        /*! \param framework Framework
         */
        UDPAssetProvider(Foundation::Framework* framework);

        //! Destructor
        virtual ~UDPAssetProvider();

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
        virtual Foundation::AssetTransferInfoVector GetTransferInfo();
        
        virtual void SetCurrentProtocolModule(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> protocolModule);

        //! Performs time-based update 
        /*! \param frametime Seconds since last frame
         */
        virtual void Update(f64 frametime);

        //! Handles incoming network packet event
        /*! \return true if event handled
         */
        bool HandleNetworkEvent(Foundation::EventDataInterface* data);

        /// Clears all transfers.
        void ClearAllTransfers();

    private:
        //! Pending asset request. Used internally by UDPAssetProvider
        struct AssetRequest
        {
            //! Asset ID
            std::string asset_id_;
            //! Asset type
            int asset_type_;
            //! Associated request tags
            RequestTagVector tags_;
        };

        //! Sends pending UDP asset requests
        /*! \param net Connected network interface
         */
        void SendPendingRequests(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net);

        //! Handles texture timeouts
        /*! \param net Connected network interface
            \param frametime Time since last frame
         */
        void HandleTextureTimeouts(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net, f64 frametime);

        //! Handles other asset timeouts
        /*! \param net Connected network interface
            \param frametime Time since last frame
         */
        void HandleAssetTimeouts(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net, f64 frametime);

        //! Makes current transfers into pending requests & clears transfers.
        /*! Called when connection lost.
         */
        void MakeTransfersPending();

        //! Stores completed asset to asset service's cache
        void StoreAsset(UDPAssetTransfer& transfer);

        //! Handles texture header message
        /*! \param msg Message
         */
        void HandleTextureHeader(ProtocolUtilities::NetInMessage* msg);

        //! Handles texture data message
        /*! \param msg Message
         */
        void HandleTextureData(ProtocolUtilities::NetInMessage* msg);

        //! Handles texture transfer abort message
        /*! \param msg Message
         */
        void HandleTextureCancel(ProtocolUtilities::NetInMessage* msg);

        //! Handles other asset transfer header message
        /*! \param msg Message
         */
        void HandleAssetHeader(ProtocolUtilities::NetInMessage* msg);
        
        //! Handles other asset transfer data message
        /*! \param msg Message
         */
        void HandleAssetData(ProtocolUtilities::NetInMessage* msg);

        //! Handles other asset transfer abort message
        /*! \param msg Message
         */
        void HandleAssetCancel(ProtocolUtilities::NetInMessage* msg);

       //! Gets asset transfer if it's in progress
        /*! \param asset_id Asset ID
            \return Pointer to transfer, or 0 if no transfer
         */
        UDPAssetTransfer* GetTransfer(const std::string& asset_id);

        //! Requests a texture from network
        /*! \param net Connected network interface
            \param asset_id Asset UUID
            \param tags Asset request tag(s)
         */
        void RequestTexture(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net,
            const RexUUID& asset_id, const RequestTagVector& tags);

        //! Requests an other asset from network
        /*! \param net Connected network interface
            \param asset_id Asset UUID
            \param tags Asset request tag(s)
         */
        void RequestOtherAsset(boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> net,
            const RexUUID& asset_id, uint asset_type, const RequestTagVector& tags);

        //! Sends progress event of asset transfer
        /*! \param transfer Asset transfer
         */
        void SendAssetProgress(UDPAssetTransfer& transfer);

        //! Sends asset transfer canceled event
        /*! \param transfer Asset transfer
         */
        void SendAssetCanceled(UDPAssetTransfer& transfer);

        typedef std::map<RexUUID, UDPAssetTransfer> UDPAssetTransferMap;

        //! Ongoing UDP asset transfers, keyed by transfer id
        UDPAssetTransferMap asset_transfers_;

        //! Ongoing UDP texture transfers, keyed by texture asset id
        UDPAssetTransferMap texture_transfers_;

        //! Asset event category
        event_category_id_t event_category_;

        //! Current asset transfer timeout
        f64 asset_timeout_;

        //! Default asset transfer timeout 
        static const float DEFAULT_ASSET_TIMEOUT;

        //! Framework
        Foundation::Framework* framework_;

        //! Pending asset requests
        typedef std::vector<AssetRequest> AssetRequestVector;
        AssetRequestVector pending_requests_;

        //! Current Protocol Module
        boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> protocolModule_;
    };
}

#endif
