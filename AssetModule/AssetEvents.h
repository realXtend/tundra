// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetEvents_h
#define incl_Asset_AssetEvents_h

#include "EventDataInterface.h"
#include "AssetInterface.h"

namespace Asset
{
    //! Asset events
    namespace Events
    {
        //! Asset ready event id
        /*! \ingroup AssetModuleClient
         */
        static const Core::event_id_t ASSET_READY = 1;
        
        //! Asset progress event id
        /*! \ingroup AssetModuleClient
         */
        static const Core::event_id_t ASSET_PROGRESS = 2;

        //! Asset transfer canceled event id
        /*! \ingroup AssetModuleClient
         */
        static const Core::event_id_t ASSET_CANCELED = 3;

        //! Asset ready event data
        /*! \ingroup AssetModuleClient
         */
        class AssetReady : public Foundation::EventDataInterface
        {
        public:
            AssetReady(const std::string& asset_id, Core::asset_type_t asset_type, Foundation::AssetPtr asset) :
                asset_id_(asset_id),
                asset_type_(asset_type),
                asset_(asset)
            {
            }
            
            virtual ~AssetReady()
            {
            }
        
            std::string asset_id_;
            Core::asset_type_t asset_type_;
            Foundation::AssetPtr asset_;
        };

        //! Asset progress event data
        /*! \ingroup AssetModuleClient
         */
        class AssetProgress : public Foundation::EventDataInterface
        {
        public:
            AssetProgress(const std::string& asset_id, Core::asset_type_t asset_type, Core::uint size, Core::uint received, Core::uint received_continuous) :
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
        
            std::string asset_id_;
            Core::asset_type_t asset_type_;
            Core::uint size_;
            Core::uint received_;
            Core::uint received_continuous_;
        };

        //! Asset transfer canceled event data
        /*! \ingroup AssetModuleClient
         */
        class AssetCanceled : public Foundation::EventDataInterface
        {
        public:
            AssetCanceled(const std::string& asset_id, Core::asset_type_t asset_type) :
                asset_id_(asset_id),
                asset_type_(asset_type)
            {
            }
            
            virtual ~AssetCanceled()
            {
            }
        
            std::string asset_id_;
            Core::asset_type_t asset_type_;
        };
    }
}
    
#endif