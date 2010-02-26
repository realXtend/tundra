// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreAssetEditorModule.cpp
 *  @brief  OgreAssetEditorModule.provides editing and previewing tools for
 *          OGRE assets such as meshes and material scripts.
 */

#include "StableHeaders.h"
#include "OgreAssetEditorModule.h"
#include "OgreScriptEditor.h"
#include "MaterialWizard.h"
#include "EditorManager.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiSceneManager.h"
#include "Framework.h"

#include "Inventory/InventoryEvents.h"
#include "UiModule.h"

#include <QStringList>
#include <QVector>
#include <QGraphicsProxyWidget>

namespace OgreAssetEditor
{

OgreAssetEditorModule::OgreAssetEditorModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_OgreAssetEditor),
    inventoryEventCategory_(0),
    assetEventCategory_(0),
    resourceEventCategory_(0),
    materialWizard_(0),
    editorManager_(0)
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
    editorManager_ = new EditorManager();

    boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (ui_module.get())
        QObject::connect(ui_module->GetSceneManager(), SIGNAL(UiStateChangeDisconnected()), editorManager_, SLOT(DeleteAll()));
}

void OgreAssetEditorModule::Uninitialize()
{
    SAFE_DELETE(materialWizard_);
    SAFE_DELETE(editorManager_);
    eventManager_.reset();
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
        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_DOWNLOADED)
        {
            Inventory::InventoryItemDownloadedEventData *downloaded = checked_static_cast<Inventory::InventoryItemDownloadedEventData *>(data);
            asset_type_t at = downloaded->assetType;
            switch(at)
            {
            case RexTypes::RexAT_ParticleScript:
            case RexTypes::RexAT_MaterialScript:
            {
                // Create new editor if it doesn't already exist.
                QString id = downloaded->inventoryId.ToString().c_str();
                if (!editorManager_->Exists(id, at))
                {
                    OgreScriptEditor *editor = new OgreScriptEditor(framework_, id, at, downloaded->name.c_str());

                    QObject::connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                        editorManager_, SLOT(Delete(const QString &, asset_type_t)));

                    editorManager_->Add(id, at, editor);
                    editor->HandleAssetReady(downloaded->asset);
                }
                else
                {
                    // Bring editor to front
                    QWidget *editor = editorManager_->GetEditor(id, at);
                    if (editor)
                    {
                        boost::shared_ptr<UiServices::UiModule> ui_module = 
                        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
                        if (ui_module)
                            ui_module->GetSceneManager()->BringProxyToFront(editor);
                    }
                }

                downloaded->handled = true;

                // Surpress this event.
                return true;
            }
            default:
                return false;
            }
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
