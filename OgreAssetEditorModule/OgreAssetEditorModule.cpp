/**
 * For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreAssetEditorModule.cpp
 *  @brief  OgreAssetEditorModule.provides editing and previewing tools for
 *          OGRE assets such as meshes and material scripts.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreAssetEditorModule.h"
#include "OgreScriptEditor.h"
#include "MaterialWizard.h"
#include "EditorManager.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "Framework.h"
#include "Inventory/InventoryEvents.h"
#include "NetworkEvents.h"
#include "TexturePreviewEditor.h"
#include "Inventory/InventoryEvents.h"
#include "ResourceInterface.h"
#include "AssetInterface.h"

#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/View/UiProxyWidget.h"

#include <QStringList>
#include <QVector>
#include <QGraphicsProxyWidget>

#include "MemoryLeakCheck.h"

namespace Naali
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

    uiModule_ = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices);
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

bool OgreAssetEditorModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
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
        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_OPEN)
        {
            // Inventory item requested for opening. Check if we have editor for its type.
            if (uiModule_.expired())
                return false;

            Inventory::InventoryItemOpenEventData *open_item = dynamic_cast<Inventory::InventoryItemOpenEventData *>(data);
            if(!open_item)
                return false;

            /*switch(open_item->assetType)
            {
                case RexTypes::RexAT_Texture:
                {
                    open_item->overrideDefaultHandler = true;
                    break;
                }
            }*/
        }
        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_DOWNLOADED)
        {
            if (uiModule_.expired())
                return false;

            // Asset downloaded, pass asset data to the right editor and bring it to front.
            // This event is sent also when asset is already in cache and doens't need to be
            // downloaded. Hence, if there is no editor created yet, create it now.
            Inventory::InventoryItemDownloadedEventData *downloaded = checked_static_cast<Inventory::InventoryItemDownloadedEventData *>(data);
            assert(downloaded);
            asset_type_t at = downloaded->assetType;
            switch(at)
            {
            case RexTypes::RexAT_ParticleScript:
            case RexTypes::RexAT_MaterialScript:
            {
                const QString &id = downloaded->inventoryId.ToString().c_str();
                const QString &name = downloaded->name.c_str();
                if (!editorManager_->Exists(id, at))
                {
                    // Editor not created, create it now.
                    OgreScriptEditor *editor = new OgreScriptEditor(framework_, id, at, name);
                    connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                        editorManager_, SLOT(Delete(const QString &, asset_type_t)));
                    editorManager_->Add(id, at, editor);
                    editor->HandleAssetReady(downloaded->asset);

                    // Add widget to scene, show and bring to front
                    UiServices::UiProxyWidget *proxy = uiModule_.lock()->GetInworldSceneController()->AddWidgetToScene(
                        editor, UiServices::UiWidgetProperties("Ogre Script Editor", UiServices::SceneWidget));
                    proxy->show();
                    uiModule_.lock()->GetInworldSceneController()->BringProxyToFront(editor);
                }
                else
                {
                    // Editor already exists, bring it to front.
                    QWidget *editor = editorManager_->GetEditor(id, at);
                    if (editor)
                        uiModule_.lock()->GetInworldSceneController()->BringProxyToFront(editor);
                }

                // Surpress this event.
                downloaded->handled = true; 
                return true;
            }
            case RexTypes::RexAT_Texture:
            {
                const QString &id = downloaded->inventoryId.ToString().c_str();
                const QString &name = downloaded->name.c_str();
                if(!editorManager_->Exists(id, at))
                {
                    TexturePreviewEditor *editor = new TexturePreviewEditor(framework_, id, at, name, downloaded->asset->GetId().c_str());
                    QObject::connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                            editorManager_, SLOT(Delete(const QString &, asset_type_t)));
                    editorManager_->Add(id, at, editor);
                    //editor->RequestTextureAsset(downloaded->asset->GetId());
                }
                else
                {
                    // Editor already exists, bring it to front.
                    QWidget *editor = editorManager_->GetEditor(id, at);
                    if (editor)
                        uiModule_.lock()->GetInworldSceneController()->BringProxyToFront(editor);
                }

                downloaded->handled = true;
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
    else if (category_id == resourceEventCategory_)
    {
        Resource::Events::ResourceReady *res = dynamic_cast<Resource::Events::ResourceReady*>(data);
        assert(res);
        if (!res)
            return false;

        QVector<QWidget*> editorList = editorManager_->GetEditorListByAssetType(RexTypes::RexAT_Texture);
        for(uint i = 0; i < editorList.size(); i++)
        {
            TexturePreviewEditor *editorWidget = qobject_cast<TexturePreviewEditor*>(editorList[i]);
            if(editorWidget)
                editorWidget->HandleResouceReady(res);
        }
    }

    return false;
}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Naali;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(OgreAssetEditorModule)
POCO_END_MANIFEST
