// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetEvents_h
#define incl_Asset_AssetEvents_h

#include "EventDataInterface.h"

namespace Asset
{
    //! asset events
    namespace Event
    {
        //! asset ready event id
        static const Core::event_id_t ASSET_READY = 1;
        
        //! asset progress event id
        static const Core::event_id_t ASSET_PROGRESS = 2;

        //! asset timeout event id
        static const Core::event_id_t ASSET_TIMEOUT = 3;

        //! asset ready event data
        class AssetReady : public Foundation::EventDataInterface
        {
        public:
            AssetReady(const std::string& asset_id, Core::asset_type_t asset_type) :
                asset_id_(asset_id),
                asset_type_(asset_type)
            {
            }
            
            virtual ~AssetReady()
            {
            }
        
            std::string asset_id_;
            Core::asset_type_t asset_type_;
        };

        //! asset progress event data
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

        //! asset timeout event data
        class AssetTimeout : public Foundation::EventDataInterface
        {
        public:
            AssetTimeout(const std::string& asset_id, Core::asset_type_t asset_type) :
                asset_id_(asset_id),
                asset_type_(asset_type)
            {
            }
            
            virtual ~AssetTimeout()
            {
            }
        
            std::string asset_id_;
            Core::asset_type_t asset_type_;
        };
    }
}
    
#endif