// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryEvents.h
 *  @brief Inventory events.
 */

#ifndef incl_Protocol_InventoryEvents_h
#define incl_Protocol_InventoryEvents_h

namespace Inventory
{
    namespace Events
    {
        static const Core::event_id_t EVENT_INVENTORY_DESCENDENT = 0x01;
        static const Core::event_id_t EVENT_INVENTORY_UPLOAD = 0x02;
    }

    enum ItemType
    {
        IIT_Folder = 0,
        IIT_Asset
    };

    class InventoryItemEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryItemEventData(const ItemType &type) : item_type(type) {}
        virtual ~InventoryItemEventData() {}
        ItemType item_type;
        RexUUID id;
        RexUUID assetId;
        RexUUID parentId;
        inventory_type_t inventoryType;
        asset_type_t assetType;
        std::string name;
        std::string description;
    };

    class InventoryUploadEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryUploadEventData() {}
        virtual ~InventoryUploadEventData() {}
        Core::StringList filenames;
    };
}

#endif // incl_Protocol_InventoryEvents_h
