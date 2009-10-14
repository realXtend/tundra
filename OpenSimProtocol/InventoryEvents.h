// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryEvents.h
 *  @brief Inventory events.
 */

#ifndef incl_InventoryEvents_h
#define incl_InventoryEvents_h

namespace OpenSimProtocol
{
    namespace InventoryEvents
    {
        static const Core::event_id_t EVENT_INVENTORY_DESCENDENT = 0x06;
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
}

#endif
