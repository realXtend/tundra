// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_UDPAssetProvider_h
#define incl_Asset_UDPAssetProvider_h

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
        
        //! Requests an asset for download
        /*! \param asset_id Asset UUID
            \param asset_type Asset type        
            \return true if asset ID was valid and download could be queued, false if not 
         */
        virtual bool RequestAsset(const std::string& asset_id, Core::asset_type_t asset_type);
        
        //! Returns whether a certain asset is already being downloaded
        virtual bool InProgress(const std::string& asset_id);
        
        //! Queries status of asset download
        /*! \param asset_id Asset UUID
            \param size Variable to receive asset size (if known, 0 if unknown)
            \param received Variable to receive amount of bytes received
            \param received_continuous Variable to receive amount of continuous bytes received from the start
            \return true If transfer in progress, and variables have been filled, false if transfer not found
         */
        virtual bool QueryAssetStatus(const std::string& asset_id, Core::uint& size, Core::uint& received, Core::uint& received_continuous);       
        
        //! Gets incomplete asset
        /*! If transfer not in progress or not enough bytes received, will return empty pointer
            
            \param asset_id Asset UUID
            \param asset_type Asset type
            \param received Minimum continuous bytes received from the start
            \return Pointer to asset
         */
        virtual Foundation::AssetPtr GetIncompleteAsset(const std::string& asset_id, Core::asset_type_t asset_type, Core::uint received);   
        
        //! Performs time-based update 
        /*! \param frametime Seconds since last frame
         */
        virtual void Update(Core::f64 frametime);     
                
        //! Handles incoming network packet event
        /*! \return true if event handled
         */
        bool HandleNetworkEvent(Foundation::EventDataInterface* data);
        
    private:
        //! Stores completed asset to asset service's cache
        void StoreAsset(AssetTransfer& transfer);
        
        //! Handles texture header message
        /*! \param msg Message
         */
        void HandleTextureHeader(NetInMessage* msg);
        
        //! Handles texture data message
        /*! \param msg Message
         */
        void HandleTextureData(NetInMessage* msg);
        
        //! Handles texture transfer abort message
        /*! \param msg Message
         */
        void HandleTextureCancel(NetInMessage* msg);
        
        //! Handles other asset transfer header message
        /*! \param msg Message
         */
        void HandleAssetHeader(NetInMessage* msg);
        
        //! Handles other asset transfer data message
        /*! \param msg Message
         */
        void HandleAssetData(NetInMessage* msg);
        
        //! Handles other asset transfer abort message
        /*! \param msg Message
         */
        void HandleAssetCancel(NetInMessage* msg);
            
       //! Gets asset transfer if it's in progress
        /*! \param asset_id Asset ID
            \return Pointer to transfer, or NULL if no transfer
         */
        AssetTransfer* GetTransfer(const std::string& asset_id);
        
        //! Requests a texture from network
        /*! \param asset_id Asset UUID
         */
        void RequestTexture(const RexTypes::RexUUID& asset_id);
        
        //! Requests an other asset from network
        /*! \param asset_id Asset UUID
         */
        void RequestOtherAsset(const RexTypes::RexUUID& asset_id, Core::uint asset_type);
        
        //! Sends progress event of asset transfer
        /*! \param transfer Asset transfer
         */
        void SendAssetProgress(AssetTransfer& transfer);

        //! Sends asset transfer canceled event
        /*! \param transfer Asset transfer
         */
        void SendAssetCanceled(AssetTransfer& transfer);    
           
        typedef std::map<RexTypes::RexUUID, AssetTransfer> AssetTransferMap;
                
        //! Ongoing UDP asset transfers, keyed by transfer id
        AssetTransferMap asset_transfers_;
        
        //! Ongoing UDP texture transfers, keyed by texture asset id
        AssetTransferMap texture_transfers_;
                        
        //! Asset event category
        Core::event_category_id_t event_category_;
        
        //! Current asset transfer timeout
        Core::f64 asset_timeout_;
        
        //! Default asset transfer timeout 
        static const Core::Real DEFAULT_ASSET_TIMEOUT;
                    
        //! Framework
        Foundation::Framework* framework_;    
    };
}

#endif