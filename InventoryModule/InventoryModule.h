// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryModule.h
 *  @brief  Inventory module. Inventory module is the owner of the inventory data model.
 *          Implement data model -spesific event handling etc. here, not in InventoryWindow
 *          or InventoryItemModel classes.
 */

#ifndef incl_InventoryModule_h
#define incl_InventoryModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "EventDataInterface.h"
#include "WorldStream.h"

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    typedef boost::weak_ptr<ProtocolModuleInterface> ProtocolWeakPtr;
}

namespace Inventory
{
    class InventoryWindow;
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

        /// Default destructor 
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

    private:
        InventoryModule(const InventoryModule &);
        void operator=(const InventoryModule &);

        void RegisterInventoryItemHandler(RexTypes::asset_type_t asset_type, QObject *handler);

        /// Event manager pointer.
        Foundation::EventManagerPtr eventManager_;

        /// Inventory event category.
        event_category_id_t inventoryEventCategory_;

        /// Network state event category.
        event_category_id_t networkStateEventCategory_;

        /// Framework event category
        event_category_id_t frameworkEventCategory_;

        /// Asset event category.
        event_category_id_t assetEventCategory_ ;

        /// Resource event category.
        event_category_id_t resourceEventCategory_;

        /// Module GUI widget
        InventoryWindow *inventoryWindow_;

        /// WorldStream pointer
        ProtocolUtilities::WorldStreamPtr currentWorldStream_ ;

        /// Inventory data model.
        InventoryPtr inventory_;

        /// Inventory data model.type.
        InventoryDataModelType inventoryType_;
    };
}

#endif
