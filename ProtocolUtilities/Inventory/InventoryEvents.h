// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryEvents.h
 *  @brief  Inventory events.
 */

#ifndef incl_Protocol_InventoryEvents_h
#define incl_Protocol_InventoryEvents_h

#include "EventDataInterface.h"
#include <RexUUID.h>
#include <AssetServiceInterface.h>

#include <QStringList>
#include <QVector>

namespace Inventory
{
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
        /// Constructor.
        /// @param type Item type.
        InventoryItemEventData(const ItemType &type) : item_type(type) {}
        /// Destructor.
        virtual ~InventoryItemEventData() {}
        /// Item type (asset or folder)
        ItemType item_type;
        /// Item ID
        RexUUID id;
        /// Asset reference ID.
        RexUUID assetId;
        /// Parent folder ID.
        RexUUID parentId;
        /// Inventory item type enumeration, see \ref RexTypes.h
        inventory_type_t inventoryType;
        /// Asset type enumeration, see \c RexTypes.h
        asset_type_t assetType;
        /// Name.
        std::string name;
        /// Description.
        std::string description;
        /// Creator ID.
        RexUUID creatorId;
        /// Owner ID.
        RexUUID ownerId;
        /// Group ID.
        RexUUID groupId;
        /// Time of creation.
        time_t creationTime;
        //! Original filename of upload, if applicable
        std::string fileName;
    };

    /// Event data class to be used with EVENT_INVENTORY_UPLOAD_FILE.
    class InventoryUploadEventData : public Foundation::EventDataInterface
    {
    public:
        /// Default constuctor.
        InventoryUploadEventData() {}
        /// Destructor.
        virtual ~InventoryUploadEventData() {}
        /// List of filenames.
        QStringList filenames;
        /// List of names for the files.
        QStringList names;
    };

    /// Event data class to be used with EVENT_INVENTORY_UPLOAD_BUFFER.
    class InventoryUploadBufferEventData : public Foundation::EventDataInterface
    {
    public:
        /// Default constuctor.
        InventoryUploadBufferEventData() {}
        /// Destructor.
        virtual ~InventoryUploadBufferEventData() {}
        /// List of filenames.
        QStringList filenames;
        /// File buffers of the files.
        QVector<QVector<u8> > buffers;
    };

    /// Event data class to be used with EVENT_INVENTORY_ITEM_OPEN.
    class InventoryItemOpenEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryItemOpenEventData() : overrideDefaultHandler(false) {}
        virtual ~InventoryItemOpenEventData() {}
        request_tag_t requestTag;
        RexUUID inventoryId;
        RexUUID assetId;
        inventory_type_t inventoryType;
        asset_type_t assetType;
        std::string name;
        bool overrideDefaultHandler;
    };

    /// Event data class to be used with EVENT_INVENTORY_ITEM_DOWNLOADED.
    class InventoryItemDownloadedEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryItemDownloadedEventData() : handled(false){}
        virtual ~InventoryItemDownloadedEventData() {}
        RexUUID inventoryId;
        Foundation::AssetPtr asset;
        request_tag_t requestTag;
        asset_type_t assetType;
        std::string name;
        bool handled;
    };
}

#endif // incl_Protocol_InventoryEvents_h
