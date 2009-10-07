// For conditions of distribution and use, see copyright notice in license.txt

// @file InventoryModule.cpp
// @brief Invetory module.

#include "StableHeaders.h"
#include "InventoryModule.h"
#include "InventoryWindow.h"
#include "../OpenSimProtocol/NetworkEvents.h"
#include "../AssetModule/AssetEvents.h"

namespace Inventory
{

InventoryModule::InventoryModule() :
    networkStateEventCategory_(0), inventoryWindow_(0),
    ModuleInterfaceImpl(Foundation::Module::MT_Inventory),
    assetEventCategory_(0)
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
    inventoryWindow_ = new InventoryWindow(framework_, 0);
    LogInfo("System " + Name() + " initialized.");
}

void InventoryModule::PostInitialize()
{
    networkStateEventCategory_ = eventManager_->QueryEventCategory("NetworkState");
    if (networkStateEventCategory_ == 0)
        LogError("Failed to query \"NetworkState\" event category");

    assetEventCategory_ = eventManager_->QueryEventCategory("Asset");
    if (assetEventCategory_ == 0)
        LogError("Failed to query \"Asset\" event category");
}

void InventoryModule::Uninitialize()
{
    SAFE_DELETE(inventoryWindow_);
    LogInfo("System " + Name() + " uninitialized.");
}

void InventoryModule::Update(Core::f64 frametime)
{
}

bool InventoryModule::HandleEvent(
    Core::event_category_id_t category_id,
    Core::event_id_t event_id,
    Foundation::EventDataInterface* data)
{
    using namespace OpenSimProtocol;

    if (category_id == networkStateEventCategory_)
    {
        if (event_id == Events::EVENT_SERVER_CONNECTED)
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
                inventoryWindow_->Show();
                break;
            default:
                break;
            }

            return false;
        }

        if (event_id == Events::EVENT_SERVER_DISCONNECTED)
        {
            inventoryWindow_->Hide();
            inventoryWindow_->ResetInventoryTreeModel();
        }

        return false;
    }

    ///\todo Handle asset uploaded events.
    //if (category_id == assetEventCategory_)

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
