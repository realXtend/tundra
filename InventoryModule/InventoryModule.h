// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryModule.h
 *  @brief Inventory module.
 */

#ifndef incl_InventoryModule_h
#define incl_InventoryModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "EventDataInterface.h"
#include "WorldStream.h"

namespace Inventory
{
    class InventoryWindow;
    class AssetUploader;
    class AbstractInventoryDataModel;

    typedef boost::shared_ptr<AssetUploader> AssetUploaderPtr;

    class InventoryModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        /// Default constructor.
        InventoryModule();

        /// Default destructor 
        virtual ~InventoryModule();

        /// ModuleInterfaceImpl overrides.
        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(Core::f64 frametime);
        bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);
        void SubscribeToNetworkEvents(boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Inventory;

        /// Upload an asset.
        Console::CommandResult UploadAsset(const Core::StringVector &params);

        /// Upload multiple assets.
        Console::CommandResult UploadMultipleAssets(const Core::StringVector &params);

        /// @return Asset uploader pointer.
//        AssetUploaderPtr GetAssetUploader() const;

        /// Get the current WorldStream
        ProtocolUtilities::WorldStreamPtr GetCurrentWorldStream() const { return currentWorldStream_ ; }

    private:
        InventoryModule(const InventoryModule &);
        void operator=(const InventoryModule &);

        /// Event manager pointer.
        Foundation::EventManagerPtr eventManager_;

        /// Inventory event category.
        Core::event_category_id_t inventoryEventCategory_;

        /// Network state event category.
        Core::event_category_id_t networkStateEventCategory_;

        /// Framework event category
        Core::event_category_id_t frameworkEventCategory_;

        /// Asset event category.
        Core::event_category_id_t assetEventCategory_ ;

        /// Resource event category.
        Core::event_category_id_t resourceEventCategory_;

        /// Module GUI widget
        InventoryWindow *inventoryWindow_;

        /// Asset uploader.
//        AssetUploaderPtr assetUploader_;

        /// WorldStream pointer
        ProtocolUtilities::WorldStreamPtr currentWorldStream_ ;

        /// Inventory data model.
        AbstractInventoryDataModel *dataModel_;
    };
}

#endif
