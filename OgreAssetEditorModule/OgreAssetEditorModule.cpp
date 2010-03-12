/**
 * For conditions of distribution and use, see copyright notice in license.txt
 *
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
#include "Inworld/InworldSceneController.h"
#include "Framework.h"

#include "Inventory/InventoryEvents.h"
#include "UiModule.h"
#include "NetworkEvents.h"

#include <QStringList>
#include <QVector>
#include <QGraphicsProxyWidget>

namespace OgreAssetEditor
{

OgreAssetEditorModule::OgreAssetEditorModule() :
    ModuleInterfaceImpl(Foundation::Module::MT_OgreAssetEditor),
    frameworkEventCategory_(0),
    inventoryEventCategory_(0),
    assetEventCategory_(0),
    resourceEventCategory_(0),
    networkStateEventCategory_(0),
    materialWizard_(0),
    editorManager_(0)
{
}

OgreAssetEditorModule::~OgreAssetEditorModule()
{
}

void OgreAssetEditorModule::Initialize()
{
    // Get event manager.
    eventManager_ = framework_->GetEventManager();
}

void OgreAssetEditorModule::PostInitialize()
{
    frameworkEventCategory_ = eventManager_->QueryEventCategory("Framework");
    if (frameworkEventCategory_ == 0)
        LogError("Failed to query \"Framework\" event category");

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
}

void OgreAssetEditorModule::Uninitialize()
{
    SAFE_DELETE(materialWizard_);
    SAFE_DELETE(editorManager_);
    eventManager_.reset();
}

void OgreAssetEditorModule::Update(f64 frametime)
{
}

bool OgreAssetEditorModule::HandleEvent(
    event_category_id_t category_id,
    event_id_t event_id,
    Foundation::EventDataInterface* data)
{
    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            networkStateEventCategory_ = eventManager_->QueryEventCategory("NetworkState");
            if (networkStateEventCategory_ == 0)
                LogError("Failed to query \"NetworkState\" event category");
            return false;
        }
    }
    if (category_id == inventoryEventCategory_)
    {
        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_DOWNLOADED)
        {
            boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
            if (!ui_module)
                return false;

            Inventory::InventoryItemDownloadedEventData *downloaded = checked_static_cast<Inventory::InventoryItemDownloadedEventData *>(data);
            assert(downloaded);
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
                        ui_module->GetInworldSceneController()->BringProxyToFront(editor);
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
    else if (category_id == networkStateEventCategory_)
    {
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
            editorManager_->DeleteAll();
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
