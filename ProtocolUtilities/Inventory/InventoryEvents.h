// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryEvents.h
 *  @brief Inventory events.
 */

#ifndef incl_Protocol_InventoryEvents_h
#define incl_Protocol_InventoryEvents_h

#include "RexUUID.h"
#include "AssetServiceInterface.h"

#include <QStringList>
#include <QVector>

namespace Inventory
{
    using namespace RexTypes;

    namespace Events
    {
        /// Event informing about new item inventory. Use this when creating new inventory items outside
        /// InventoryModule e.g. when handling incoming InventoryDescendents packet
        static const event_id_t EVENT_INVENTORY_DESCENDENT = 0x01;

        /// Send this event with appropriate event data to upload asset from file.
        static const event_id_t EVENT_INVENTORY_UPLOAD_FILE = 0x02;

        /// Send this event with appropriate event data to upload asset from buffer.
        static const event_id_t EVENT_INVENTORY_UPLOAD_BUFFER = 0x03;

        /// Event informing inventory item opening (download request).
        static const event_id_t EVENT_INVENTORY_ITEM_OPEN = 0x04;

        /// Event informing successful download of inventory asset.
        static const event_id_t EVENT_INVENTORY_ITEM_DOWNLOADED = 0x05;
    }

    /// Inventory item type enumeration.
    enum ItemType
    {
        IIT_Folder = 0,
        IIT_Asset
    };

    /// Event data class to be used with EVENT_INVENTORY_DESCENDENT.
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
        //! Original filename of upload, if applicable
        std::string fileName;
    };

    /// Event data class to be used with EVENT_INVENTORY_UPLOAD_FILE.
    class InventoryUploadEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryUploadEventData() {}
        virtual ~InventoryUploadEventData() {}
        QStringList filenames;
    };

    /// Event data class to be used with EVENT_INVENTORY_UPLOAD_BUFFER.
    class InventoryUploadBufferEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryUploadBufferEventData() {}
        virtual ~InventoryUploadBufferEventData() {}
        QStringList filenames;
        QVector<QVector<u8> > buffers;
    };

    /// Event data class to be used with EVENT_INVENTORY_ITEM_OPEN.
    class InventoryItemOpenEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryItemOpenEventData() {}
        virtual ~InventoryItemOpenEventData() {}
        request_tag_t requestTag;
        RexUUID inventoryId;
        RexUUID assetId;
        inventory_type_t inventoryType;
        asset_type_t assetType;
        std::string name;
    };

    /// Event data class to be used with EVENT_INVENTORY_ITEM_DOWNLOADED.
    class InventoryItemDownloadedEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryItemDownloadedEventData() {}
        virtual ~InventoryItemDownloadedEventData() {}
        RexUUID inventoryId;
        Foundation::AssetPtr asset;
        request_tag_t requestTag;
        asset_type_t assetType;
    };
}

#endif // incl_Protocol_InventoryEvents_h
