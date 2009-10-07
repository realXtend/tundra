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

namespace Inventory
{
    class InventoryWindow;

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
        bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        //! Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Inventory;

    private:
        InventoryModule(const InventoryModule &);
        void operator=(const InventoryModule &);

        /// Event manager pointer.
        Foundation::EventManagerPtr eventManager_;

        /// Network state event category.
        Core::event_category_id_t networkStateEventCategory_;

        /// Network state event category.
        Core::event_category_id_t assetEventCategory_;

        /// Module GUI widget
        InventoryWindow *inventoryWindow_;
    };
}

#endif
