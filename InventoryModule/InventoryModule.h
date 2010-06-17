/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryModule.h
 *  @brief  Inventory module. Inventory module is the owner of the inventory data model.
 *          Implement data model -spesific event handling etc. here, not in InventoryWindow
 *          or InventoryItemModel classes.
 */

#ifndef incl_InventoryModule_h
#define incl_InventoryModule_h

#include "InventoryModuleApi.h"
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include <QMap>

namespace ProtocolUtilities
{
    class WorldStream;
}

namespace Foundation
{
    class EventDataInterface;
}

namespace Inventory
{
    class InventoryWindow;
    class UploadProgressWindow;
    class ItemPropertiesWindow;
    class AbstractInventoryDataModel;
    typedef boost::shared_ptr<AbstractInventoryDataModel> InventoryPtr;
    class InventoryService;

    class INVENTORY_MODULE_API InventoryModule : public QObject, public Foundation::ModuleInterface
    {
        Q_OBJECT

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
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);
        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

        /// Subscribes this module to listen network events.
        void SubscribeToNetworkEvents();

        /// Console command for uploading an asset, non-threaded.
        Console::CommandResult UploadAsset(const StringVector &params);

        /// Console command for uploading multiple assets, threaded.
        Console::CommandResult UploadMultipleAssets(const StringVector &params);

#ifdef _DEBUG
        /// Console command for testing the inventory service.
        Console::CommandResult InventoryServiceTest(const StringVector &params);
#endif

        /// Returns the inventory pointer
        /// @note Meant mostly for module's internal use.
        InventoryPtr GetInventoryPtr() const { return inventory_; }

        /// Return the type of the inventory data model (e.g. OpenSim or WebDAV).
        InventoryDataModelType GetInventoryDataModelType() const { return inventoryType_; }

        /// Returns pointer to the inventory service.
        InventoryService *GetInventoryService() const { return service_; }

    public slots:
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

        /// Name of the module.
        static std::string type_name_static_;

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

        /// Inventory window.
        InventoryWindow *inventoryWindow_;

        /// Upload progress window.
//        UploadProgressWindow *uploadProgressWindow_;

        /// WorldStream pointer
        boost::shared_ptr<ProtocolUtilities::WorldStream> currentWorldStream_ ;

        /// Inventory data model.
        InventoryPtr inventory_;

        /// Inventory data model.type.
        InventoryDataModelType inventoryType_;

        /// List of item properties widgets.
        QMap<QString, ItemPropertiesWindow *> itemPropertiesWindows_;

        /// Inventory service pointer.
        InventoryService *service_;
    };
}

#endif
