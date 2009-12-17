// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreAssetEditorModule.cpp
 *  @brief  Ogre asset editor module.provides editing and previewing tools for
 *          OGRE assets such as mesh, material & particle scripts.
 */

#include "StableHeaders.h"
#include "OgreAssetEditorModule.h"
#include "OgreScriptEditor.h"
#include "MaterialWizard.h"

#include <Inventory/InventoryEvents.h>
#include <AssetEvents.h>

#include <QObject>
#include <QStringList>
#include <QVector>

namespace OgreAssetEditor
{

OgreAssetEditorModule::OgreAssetEditorModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_OgreAssetEditor),
    inventoryEventCategory_(0),
    assetEventCategory_(0),
    resourceEventCategory_(0),
    materialWizard_(0)
{
}

OgreAssetEditorModule::~OgreAssetEditorModule()
{
}

void OgreAssetEditorModule::Load()
{
    LogInfo("System " + Name() + " loaded.");
}

void OgreAssetEditorModule::Unload()
{
    LogInfo("System " + Name() + " unloaded.");
}

void OgreAssetEditorModule::Initialize()
{
    // Get event manager.
    eventManager_ = framework_->GetEventManager();

    LogInfo("System " + Name() + " initialized.");
}

void OgreAssetEditorModule::PostInitialize()
{
    inventoryEventCategory_ = eventManager_->QueryEventCategory("Inventory");
    if (inventoryEventCategory_ == 0)
        LogError("Failed to query \"Inventory\" event category");

    assetEventCategory_ = eventManager_->QueryEventCategory("Asset");
    if (assetEventCategory_ == 0)
        LogError("Failed to query \"Asset\" event category");

    resourceEventCategory_ = eventManager_->QueryEventCategory("Resource");
    if (resourceEventCategory_ == 0)
        LogError("Failed to query \"Resource\" event category");

     materialWizard_ = new MaterialWizard(framework_);
}

void OgreAssetEditorModule::Uninitialize()
{
    AssetEditorMapIter it(assetEditors_);
    while(it.hasNext())
    {
        QObject *editor = it.next().value();
        SAFE_DELETE(editor);
    }

    LogInfo("System " + Name() + " uninitialized.");
}

void OgreAssetEditorModule::Update(f64 frametime)
{
}

bool OgreAssetEditorModule::HandleEvent(
    event_category_id_t category_id,
    event_id_t event_id,
    Foundation::EventDataInterface* data)
{
    if (category_id == inventoryEventCategory_)
    {
        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_OPEN)
        {
            Inventory::InventoryItemOpenEventData *openEvent = static_cast<Inventory::InventoryItemOpenEventData *>(data);
            RexTypes::asset_type_t at = openEvent->assetType;
            switch(at)
            {
            case RexTypes::RexAT_ParticleScript:
            {
                AssetEditorMap::iterator it = assetEditors_.find(qMakePair(openEvent->inventoryId, openEvent->requestTag));
                if (it == assetEditors_.end())
                {
                    OgreScriptEditor *editor = new OgreScriptEditor(framework_, at, openEvent->name.c_str());
                    assetEditors_[qMakePair(openEvent->inventoryId, openEvent->requestTag)] = editor;
                }
                break;
            }
            case RexTypes::RexAT_MaterialScript:
            {
                AssetEditorMap::iterator it = assetEditors_.find(qMakePair(openEvent->inventoryId, openEvent->requestTag));
                if (it == assetEditors_.end())
                {
                    OgreScriptEditor *editor = new OgreScriptEditor(framework_, at, openEvent->name.c_str());
                    assetEditors_[qMakePair(openEvent->inventoryId, openEvent->requestTag)] = editor;
                }
                break;
            }
            default:
                break;
            }

            return false;
        }

        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_DOWNLOADED)
        {
            Inventory::InventoryItemDownloadedEventData *downloaded = static_cast<Inventory::InventoryItemDownloadedEventData *>(data);
            AssetEditorMap::iterator it = assetEditors_.find(qMakePair(downloaded->inventoryId, downloaded->requestTag));
            if (it != assetEditors_.end())
                dynamic_cast<OgreScriptEditor *>(it.value())->HandleAssetReady(downloaded->asset);
        }
    }

    return false;
}

} // namespace OgreAssetEditor

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace OgreAssetEditor;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(OgreAssetEditorModule)
POCO_END_MANIFEST
