// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InventoryEvents_h
#define incl_InventoryEvents_h

#include "EventDataInterface.h"
//#include "InventoryModel.h"

namespace Inventory
{
    /// Defines the events posted by the OpenSimProtocolModule in category <b>NetworkState</b>.
    namespace Events
    {
        ///
        static const Core::event_id_t EVENT_CREATE_INVENTORY = 0x01;

        ///
        static const Core::event_id_t EVENT_DELETE_INVENTORY = 0x02;

        ///
        static const Core::event_id_t EVENT_CREATE_FOLDER = 0x03;

        ///
        static const Core::event_id_t EVENT_DELETE_FOLDER = 0x04;

        ///
        static const Core::event_id_t EVENT_MOVE_FOLDER = 0x05;

        ///
        static const Core::event_id_t EVENT_CREATE_ASSET = 0x06;

        ///
        static const Core::event_id_t EVENT_DELETE_ASSET = 0x07;

        ///
        static const Core::event_id_t EVENT_MOVE_ASSET = 0x08;

    }

    /// Event data interface for inbound messages.
    class InventoryFolderEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryFolderEventData() {}
        virtual ~InventoryFolderEventData() {}
        // InventoryFolder pointer
    };

    /// Event data interface for inbound messages.
    class InventoryAssetEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryAssetEventData() {}
        virtual ~InventoryAssetEventData() {}
        // InventoryAsset pointer.
    };
}

#endif
