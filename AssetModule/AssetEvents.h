// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetEvents_h
#define incl_Asset_AssetEvents_h

namespace Asset
{
    //! asset events
    namespace Event
    {
        //! asset ready event id
        static const Core::event_id_t ASSET_READY = 1;
        
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
    }
}
    
#endif