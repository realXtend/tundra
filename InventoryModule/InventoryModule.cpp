// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryModule.cpp
 *  @brief Inventory module.
 */

#include "StableHeaders.h"
#include "InventoryModule.h"
#include "RexLogicModule.h"
#include "InventoryWindow.h"
#include "NetworkEvents.h"
#include "InventoryEvents.h"

namespace Inventory
{

InventoryModule::InventoryModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_Inventory),
    networkStateEventCategory_(0), inventoryWindow_(0)
{
}

// virtual
InventoryModule::~InventoryModule()
{
}

void InventoryModule::Load()
{
    LogInfo("System " + Name() + " loaded.");
}

void InventoryModule::Unload()
{
    LogInfo("System " + Name() + " unloaded.");
}

void InventoryModule::Initialize()
{
    eventManager_ = framework_->GetEventManager();
    inventoryEventCategory_ = eventManager_->RegisterEventCategory("Inventory");

    rexLogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(
        Foundation::Module::MT_WorldLogic).lock().get());

    boost::shared_ptr<Console::CommandService> console = framework_->GetService<Console::CommandService>
        (Foundation::Service::ST_ConsoleCommand).lock();
    if (console)
    {
        console->RegisterCommand(Console::CreateCommand("Upload",
            "Upload an asset. Usage: Upload(AssetType, Name, Description)",
            Console::Bind(rexLogic_, &RexLogic::RexLogicModule::UploadAsset)));

        console->RegisterCommand(Console::CreateCommand("MultiUpload",
            "Upload multiple assets.",
            Console::Bind(rexLogic_, &RexLogic::RexLogicModule::UploadMultipleAssets)));
    }

    inventoryWindow_ = new InventoryWindow(framework_, rexLogic_);

    LogInfo("System " + Name() + " initialized.");
}

void InventoryModule::PostInitialize()
{
    networkStateEventCategory_ = eventManager_->QueryEventCategory("NetworkState");
    if (networkStateEventCategory_ == 0)
        LogError("Failed to query \"NetworkState\" event category");
}

void InventoryModule::Uninitialize()
{
    SAFE_DELETE(inventoryWindow_);
    LogInfo("System " + Name() + " uninitialized.");
}

void InventoryModule::Update(Core::f64 frametime)
{
}

bool InventoryModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id,
    Foundation::EventDataInterface* data)
{
    if (category_id == networkStateEventCategory_)
    {
        using namespace OpenSimProtocol;

        if (event_id == OpenSimProtocol::Events::EVENT_SERVER_CONNECTED)
        {
            AuthenticationEventData *auth_data = dynamic_cast<AuthenticationEventData *>(data);
            if (!auth_data)
                return false;

            switch(auth_data->type)
            {
            case AT_Taiga:
                //inventoryWindow_->InitWebDavInventoryTreeModel();
                break;
            case AT_OpenSim:
            case AT_RealXtend:
                inventoryWindow_->InitOpenSimInventoryTreeModel();
                break;
            default:
                break;
            }

            return false;
        }

        if (event_id == OpenSimProtocol::Events::EVENT_SERVER_DISCONNECTED)
        {
            inventoryWindow_->ResetInventoryTreeModel();
            inventoryWindow_->Hide();
        }

        return false;
    }

    if (category_id == inventoryEventCategory_)
    {
        if (event_id == Inventory::Events::EVENT_INVENTORY_DESCENDENT)
        {
            InventoryItemEventData *item_data = dynamic_cast<InventoryItemEventData *>(data);
            if (!item_data)
                return false;

            inventoryWindow_->HandleInventoryDescendent(item_data);
        }

        return false;
    }

    return false;
}

} // namespace Inventory

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Inventory;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(InventoryModule)
POCO_END_MANIFEST
