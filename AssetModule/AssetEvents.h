// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetEvents_h
#define incl_Asset_AssetEvents_h

#include "IEventData.h"
#include "AssetInterface.h"

namespace Asset
{
    //! Asset events
    namespace Events
    {
        //! Sent when an asset has been completely loaded. Uses AssetReady event data structure.
        /*! \ingroup AssetModuleClient
         */
        static const event_id_t ASSET_READY = 1;
        
        //! Sent periodically to tell about the progress of an asset download. Uses AssetProgress event data structure.
        /*! \ingroup AssetModuleClient
         */
        static const event_id_t ASSET_PROGRESS = 2;

        //! Sent when asset download has been canceled out of some reason (for example network timeout). Uses AssetCanceled event data structure.
        /*! \ingroup AssetModuleClient
         */
        static const event_id_t ASSET_CANCELED = 3;

        //! Sent when asset wants to be open. 
 
        static const event_id_t ASSET_OPEN = 4;

        //! Asset ready event data
        /*! \ingroup AssetModuleClient
         */
        class AssetReady : public IEventData
        {
        public:
            AssetReady(const std::string& asset_id, const std::string& asset_type, Foundation::AssetPtr asset, request_tag_t tag) :
                asset_id_(asset_id),
                asset_type_(asset_type),
                asset_(asset),
                tag_(tag)
            {
            }
            
            virtual ~AssetReady()
            {
            }
            
            //! ID of the ready asset
            std::string asset_id_;
            //! Type of the ready asset
            std::string asset_type_;
            //! Pointer to the ready asset
            Foundation::AssetPtr asset_;
            //! Request tag, identifying the asset request
            request_tag_t tag_;
        };

        //! Asset progress event data
        /*! \ingroup AssetModuleClient
         */
        class AssetProgress : public IEventData
        {
        public:
            AssetProgress(const std::string& asset_id, const std::string& asset_type, uint size, uint received, uint received_continuous) :
                asset_id_(asset_id),
                asset_type_(asset_type),
                size_(size),
                received_(received),
                received_continuous_(received_continuous)
            {
            }
            
            virtual ~AssetProgress()
            {
            }
            
            //! ID of the asset in progress
            std::string asset_id_;
            //! Type of the asset in progress
            std::string asset_type_;
            //! Total size in bytes of the asset
            uint size_;
            //! Total received bytes (may be non-contiguous) for the asset so far
            uint received_;
            //! Total received continuous bytes from the asset data beginning so far
            uint received_continuous_;
        };

        //! Asset transfer canceled event data
        /*! \ingroup AssetModuleClient
         */
        class AssetCanceled : public IEventData
        {
        public:
            AssetCanceled(const std::string& asset_id, const std::string& asset_type) :
                asset_id_(asset_id),
                asset_type_(asset_type)
            {
            }
            
            virtual ~AssetCanceled()
            {
            }
        
            //! ID of the asset whose transfer was canceled
            std::string asset_id_;
            //! Type of the asset whose transfer was canceled
            std::string asset_type_;
        };

        /**
         * Asset open event data. 
         * @ingroup AssetModule
         */
        class AssetOpen : public IEventData
        {
            public: 
                AssetOpen(const QString& asset_id, const QString& asset_type) : asset_id_(asset_id), asset_type_(asset_type) {}
                virtual ~AssetOpen() {}

                /// ID of the asset which is wanted to open
                QString asset_id_;
                /// Type of the asset which is wanted to open
                QString asset_type_;
        };

    }
}
    
#endif