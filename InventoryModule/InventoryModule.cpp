// For conditions of distribution and use, see copyright notice in license.txt

// @file InventoryModule.cpp
// @brief Invetory module.

#include "StableHeaders.h"
#include "InventoryModule.h"

namespace Inventory
{

InventoryModule::InventoryModule() :
    inventoryEventCategory_(0), inventoryWindow_(0),
    ModuleInterfaceImpl(Foundation::Module::MT_Inventory)
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
    LogInfo("System " + Name() + " initialized.");
	inventoryWindow_ = new InventoryWindow(framework_, 0);
	
}

void InventoryModule::PostInitialize()
{
    eventManager_ = framework_->GetEventManager();
    inventoryEventCategory_ = eventManager_->QueryEventCategory("Inventory");
    if (inventoryEventCategory_ == 0)
        LogError("Failed to query \"Inventory\" event category");
}

void InventoryModule::Uninitialize()
{
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
    if (category_id != inventoryEventCategory_)
        return false;

    switch(event_id)
    {
    case Events::EVENT_CREATE_INVENTORY:
    case Events::EVENT_DELETE_INVENTORY:
    case Events::EVENT_CREATE_FOLDER:
    case Events::EVENT_DELETE_FOLDER:
    case Events::EVENT_MOVE_FOLDER:
    case Events::EVENT_CREATE_ASSET:
    case Events::EVENT_DELETE_ASSET:
    case Events::EVENT_MOVE_ASSET:
    default:
       break;
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
