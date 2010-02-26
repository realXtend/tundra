// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryModule.h
 *  @brief  Inventory module. Inventory module is the owner of the inventory data model.
 *          Implement data model -spesific event handling etc. here, not in InventoryWindow
 *          or InventoryItemModel classes.
 *
 *          It's not recommended to get any pointers to InventoryModule from other modules
 *          because InventoryModule is designed to be an optional module.
 */

#ifndef incl_InventoryModule_h
#define incl_InventoryModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "WorldStream.h"

#include <QMap>

namespace Foundation
{
    class EventDataInterface;
}

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    typedef boost::weak_ptr<ProtocolModuleInterface> ProtocolWeakPtr;
}

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Inventory
{
    class InventoryWindow;
    class UploadProgressWindow;
    class ItemPropertiesWindow;
    class AbstractInventoryDataModel;
    typedef boost::shared_ptr<AbstractInventoryDataModel> InventoryPtr;

    class InventoryModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        /// Enumeration of inventory data models.
        enum InventoryDataModelType
        {
            IDMT_OpenSim,
            IDMT_WebDav,
            IDMT_Unknown
        };

        /// Default constructor.
        InventoryModule();

        /// Destructor 
        ~InventoryModule();

        /// ModuleInterfaceImpl overrides.
        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(
            event_category_id_t category_id,
            event_id_t event_id,
            Foundation::EventDataInterface* data);

        void SubscribeToNetworkEvents(ProtocolUtilities::ProtocolWeakPtr currentProtocolModule);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Inventory;

        /// Console command for uploading an asset, non-threaded.
        Console::CommandResult UploadAsset(const StringVector &params);

        /// Console command for uploading multiple assets, threaded.
        Console::CommandResult UploadMultipleAssets(const StringVector &params);

        /// Get the current WorldStream
        ProtocolUtilities::WorldStreamPtr GetCurrentWorldStream() const { return currentWorldStream_ ; }

        /// Returns the inventory pointer
        InventoryPtr GetInventoryPtr() const { return inventory_; }

        /// Return the type of the inventory data model (e.g. OpenSim or WebDAV).
        InventoryDataModelType GetInventoryDataModelType() const { return inventoryType_; }

        /// Opens new item properties window.
        /// @param inventory_id Inventory ID of the item.
        void OpenItemPropertiesWindow(const QString &inventory_id);

        /// Notifies server if item properties were modified.
        /// The actual window takes care of closing and deleting itself.
        /// @param inventory_id Inventory ID of the item.
        /// @param save_changes Do we want to save changes.
        void CloseItemPropertiesWindow(const QString &inventory_id, bool save_changes = false);

    private:
        Q_DISABLE_COPY(InventoryModule);

        /// Handles InventoryDescendents packet.
        /// @param data Event data.
        void HandleInventoryDescendents(Foundation::EventDataInterface* event_data);

        /// Handles CreateInventoryItem packet.
        /// @param data Event data.
        void HandleUpdateCreateInventoryItem(Foundation::EventDataInterface* event_data);

        /// Handles CreateInventoryItem packet.
        /// @param data Event data.
        void HandleUuidNameReply(Foundation::EventDataInterface* event_data);

        /// Handles CreateInventoryItem packet.
        /// @param data Event data.
        void HandleUuidGroupNameReply(Foundation::EventDataInterface* event_data);

        /// Deletes all item properties windows.
        void DeleteAllItemPropertiesWindows();

        /// Connects upload-related signals.
        void ConnectSignals();

        /// Event manager pointer.
        Foundation::EventManagerPtr eventManager_;

        /// Inventory event category.
        event_category_id_t inventoryEventCategory_;

        /// NetworkState event category.
        event_category_id_t networkStateEventCategory_;

        /// NetworkIn event category.
        event_category_id_t networkInEventCategory_;

        /// Framework event category
        event_category_id_t frameworkEventCategory_;

        /// Asset event category.
        event_category_id_t assetEventCategory_ ;

        /// Resource event category.
        event_category_id_t resourceEventCategory_;

        /// Module GUI widget
        InventoryWindow *inventoryWindow_;

        /// Upload progress window.
        UploadProgressWindow *uploadProgressWindow_;

        /// WorldStream pointer
        ProtocolUtilities::WorldStreamPtr currentWorldStream_ ;

        /// Inventory data model.
        InventoryPtr inventory_;

        /// Inventory data model.type.
        InventoryDataModelType inventoryType_;

        /// List of item properties widgets.
        QMap<QString, ItemPropertiesWindow *> itemPropertiesWindows_;
    };
}

#endif
