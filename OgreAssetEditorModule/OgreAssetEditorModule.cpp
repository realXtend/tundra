//$ HEADER_MOD_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreAssetEditorModule.cpp
 *  @brief  OgreAssetEditorModule.provides editing and previewing tools for
 *          OGRE assets such as meshes and material scripts.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "OgreAssetEditorModule.h"
#include "EditorManager.h"
#include "OgreScriptEditor.h"
#include "TexturePreviewEditor.h"
#include "AudioPreviewEditor.h"
#include "MeshPreviewEditor.h"
#include "MaterialWizard.h"

#include "Framework.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "NetworkEvents.h"
#include "Inventory/InventoryEvents.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UiProxyWidget.h"
#include "IAsset.h"
#include "OgreConversionUtils.h"

#include "MemoryLeakCheck.h"

EditorAction::EditorAction(const AssetPtr &asset, const QString &text, QMenu *menu) : QAction(text, menu)
{
    this->asset = asset;
}

std::string OgreAssetEditorModule::typeNameStatic = "OgreAssetEditor";

OgreAssetEditorModule::OgreAssetEditorModule() :
    IModule(typeNameStatic),
    frameworkEventCategory(0),
    inventoryEventCategory(0),
    networkStateEventCategory(0),
    materialWizard(0),
    editorManager(0)
{
}

OgreAssetEditorModule::~OgreAssetEditorModule()
{
}

void OgreAssetEditorModule::Initialize()
{
}

void OgreAssetEditorModule::PostInitialize()
{
    framework_->GetEventManager()->RegisterEventSubscriber(this, 99);

    frameworkEventCategory = framework_->GetEventManager()->QueryEventCategory("Framework");
    inventoryEventCategory = framework_->GetEventManager()->QueryEventCategory("Inventory");

    materialWizard = new MaterialWizard;
    connect(materialWizard, SIGNAL(NewMaterial(Inventory::InventoryUploadEventData *)),
        this, SLOT(UploadFile(Inventory::InventoryUploadEventData *)));

/*
    uiService_ = framework_->GetServiceManager()->GetService<UiServiceInterface>(Service::ST_Gui);
    if (!uiService_.expired())
    {
        UiProxyWidget *proxy  = uiService_.lock()->AddWidgetToScene(materialWizard_, true, true);
        uiService_.lock()->AddWidgetToMenu(materialWizard_, tr("Material Wizard"), tr("View"),
            "./data/ui/images/menus/edbutton_MATWIZ_normal.png");
		//$ BEGIN_MOD $
        //connect(proxy, SIGNAL(Closed()), materialWizard_, SLOT(Close()));
		//$ END_MOD $  
    }
*/
    editorManager = new EditorManager;

    connect(framework_->Ui(), SIGNAL(ContextMenuAboutToOpen(QMenu *, QList<QObject *>)), SLOT(OnContextMenuAboutToOpen(QMenu *, QList<QObject *>)));
}

void OgreAssetEditorModule::Uninitialize()
{
    SAFE_DELETE(materialWizard);
    SAFE_DELETE(editorManager);
}

void OgreAssetEditorModule::Update(f64 frametime)
{
    RESETPROFILER;
}

bool OgreAssetEditorModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
/*
    if (category_id == frameworkEventCategory)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            networkStateEventCategory = framework_->GetEventManager()->QueryEventCategory("NetworkState");
            return false;
        }
    }

    if ( category_id == assetEventCategory_ )
    {
        if (event_id == Asset::Events::ASSET_OPEN)
        {
            // Search first editor
            Asset::Events::AssetOpen* open = static_cast<Asset::Events::AssetOpen* >(data);
            int asset_type = open->asset_type_.toUInt();
            switch ( asset_type ) 
            {
            
            case RexTypes::RexAT_Mesh:
                {
                  QString id = open->asset_id_;
                  QString name = open->asset_type_;
                  
                  if(!editorManager_->Exists(id, name.toUInt()))
                    {
                        MeshPreviewEditor *editor = new MeshPreviewEditor(framework_);
                        connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                                editorManager_, SLOT(Delete(const QString &, asset_type_t)));
                        editorManager_->Add(id, name.toUInt(), editor);
                        editor->Open(id, name);
                        return true;
                    }
                    else
                    {
                        // Editor already exists, bring it to front.

                        QWidget *editor = editorManager_->GetEditor(id, name.toUInt());
                        if (editor != 0)
                        {
                            uiService_.lock()->BringWidgetToFront(editor);
                            MeshPreviewEditor *editorWidget = qobject_cast<MeshPreviewEditor*>(editor);
                            if ( editorWidget != 0)
                                editorWidget->Open(id, name);
                           
                            return true;
                        }
                    }

                    break;
               
                }
            case RexTypes::RexAT_Texture:
                {
                  QString id = open->asset_id_;
                  QString type = open->asset_type_;
                  
                  if(!editorManager_->Exists(id, type.toUInt()))
                    {
                        TexturePreviewEditor *editor = new TexturePreviewEditor(framework_);
                        connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                            editorManager_, SLOT(Delete(const QString &, asset_type_t)));
                        editorManager_->Add(id, type.toUInt(), editor);
                        editor->RequestTextureAsset(id);
                        return true;
                    }
                    else
                    {
                        // Editor already exists, bring it to front.
                        QWidget *editor = editorManager_->GetEditor(id, type.toUInt());
                        if (editor != 0)
                        {
                            uiService_.lock()->BringWidgetToFront(editor);
                            return true;
                        }
                   }
                    break;
                }
            default:
                break;
            }
        }
    }

    if (category_id == inventoryEventCategory)
    {
        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_OPEN)
        {
            // Inventory item requested for opening. Check if we have editor for its type.
            //if (uiService_.expired())
              //  return false;

            Inventory::InventoryItemOpenEventData *open_item = dynamic_cast<Inventory::InventoryItemOpenEventData *>(data);
            if(!open_item)
                return false;
        }
        if (event_id == Inventory::Events::EVENT_INVENTORY_ITEM_DOWNLOADED)
        {
            //if (uiService_.expired())
              //  return false;

            // Asset downloaded, pass asset data to the right editor and bring it to front.
            // This event is sent also when asset is already in cache and doesn't need to be
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
                if (!editorManager->Exists(id, at))
                {
                    // Editor not created, create it now.
                    OgreScriptEditor *editor = new OgreScriptEditor(id, at, name);
                    connect(editor, SIGNAL(UploadNewScript(Inventory::InventoryUploadBufferEventData *)),
                        this, SLOT(UploadBuffer(Inventory::InventoryUploadBufferEventData *)));
                    connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                        editorManager, SLOT(Delete(const QString &, asset_type_t)));
                    editorManager->Add(id, at, editor);
//                    editor->HandleAssetReady(downloaded->asset);

                    // Add widget to scene, show and bring to front
                    //uiService_.lock()->AddWidgetToScene(editor);
                    //uiService_.lock()->ShowWidget(editor);
                    //uiService_.lock()->BringWidgetToFront(editor);
                }
                else
                {
                    // Editor already exists, bring it to front.
                    //QWidget *editor = editorManager_->GetEditor(id, at);
                    //if (editor)
                        //uiService_.lock()->BringWidgetToFront(editor);
                }
                // Suppress this event.
                downloaded->handled = true; 
                return true;
            }
            case RexTypes::RexAT_SoundVorbis:
            case RexTypes::RexAT_SoundWav:
            {
                const QString &id = downloaded->inventoryId.ToString().c_str();
                const QString &name = downloaded->name.c_str();
                if(!editorManager->Exists(id, at))
                {
                    AudioPreviewEditor *editor = new AudioPreviewEditor(framework_, id, at, name);
                    connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                        editorManager, SLOT(Delete(const QString &, asset_type_t)));
                    editorManager->Add(id, at, editor);
//                    editor->HandleAssetReady(downloaded->asset);
                }
                else
                {
                    // Editor already exists, bring it to front.
                    QWidget *editor = editorManager->GetEditor(id, at);
                    if (editor)
                    {
                        //uiService_.lock()->BringWidgetToFront(editor);
                        AudioPreviewEditor *audioWidget = qobject_cast<AudioPreviewEditor*>(editor);
                        if(audioWidget)
                            audioWidget->HandleAssetReady(downloaded->asset);
                    }
                }

                downloaded->handled = true;
                return true;
            }
            case RexTypes::RexAT_Mesh:
            {
                const QString &id = downloaded->inventoryId.ToString().c_str();
                const QString &name = downloaded->name.c_str();
                if(!editorManager_->Exists(id, at))
                {
                    MeshPreviewEditor *editor = new MeshPreviewEditor(framework_, id, at, name, downloaded->asset->GetId().c_str());
                    QObject::connect(editor, SIGNAL(Closed(const QString &, asset_type_t)),
                            editorManager_, SLOT(Delete(const QString &, asset_type_t)));
                    editorManager_->Add(id, at, editor);
                    //editor->HandleAssetReady(downloaded->asset);
                }
                else
                {
                    // Editor already exists, bring it to front.
                    QWidget *editor = editorManager_->GetEditor(id, at);
                    if (editor != 0)
                    {
                        uiService_.lock()->BringWidgetToFront(editor);
                        MeshPreviewEditor *editorWidget = qobject_cast<MeshPreviewEditor*>(editor);
                        if(editorWidget)
                            editorWidget->RequestMeshAsset(downloaded->asset->GetId().c_str());
                    }
                }

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
                }
                else
                {
                    // Editor already exists, bring it to front.
                    QWidget *editor = editorManager_->GetEditor(id, at);
                    if (editor)
                        uiService_.lock()->BringWidgetToFront(editor);
                }

                downloaded->handled = true;
                return true;
            }
            default:
                return false;
            }
        }
    }
    else if (category_id == networkStateEventCategory)
    {
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
            editorManager->DeleteAll();
    }
    else if (category_id == resourceEventCategory_)
    {
        if (event_id == Resource::Events::RESOURCE_CANCELED)
            return false;

        Resource::Events::ResourceReady *res = dynamic_cast<Resource::Events::ResourceReady*>(data);
        assert(res);
        if (!res)
            return false;

        if(res->resource_->GetType() == "Texture")
        {
            QVector<QWidget*> editorList = editorManager_->GetEditorListByAssetType(RexTypes::RexAT_Texture);
            for(uint i = 0; i < editorList.size(); i++)
            {
                TexturePreviewEditor *editorWidget = qobject_cast<TexturePreviewEditor*>(editorList[i]);
                if(editorWidget)
                    editorWidget->HandleResouceReady(res);
            }
        }
        else if(res->resource_->GetType() == "Sound")
        {
            QVector<QWidget*> editorList = editorManager_->GetEditorListByAssetType(RexTypes::RexAT_SoundVorbis);
            for(uint i = 0; i < editorList.size(); i++)
            {
                AudioPreviewEditor *editorWidget = qobject_cast<AudioPreviewEditor*>(editorList[i]);
                if(editorWidget)
                    editorWidget->HandleResouceReady(res);
            }
        }

        if ( res->resource_->GetType() == "Mesh" || res->resource_->GetType() == "OgreMesh")
        {
            QVector<QWidget*> editorList = editorManager_->GetEditorListByAssetType(RexTypes::RexAT_Mesh);
            for(uint i = 0; i < editorList.size(); i++)
            {
                MeshPreviewEditor *editorWidget = qobject_cast<MeshPreviewEditor*>(editorList[i]);
                if(editorWidget != 0)
                    editorWidget->HandleResouceReady(res);
            }
        }
    }
*/
    return false;
}

void OgreAssetEditorModule::UploadFile(Inventory::InventoryUploadEventData *data)
{
    framework_->GetEventManager()->SendEvent(inventoryEventCategory, Inventory::Events::EVENT_INVENTORY_UPLOAD_FILE, data);
}

void OgreAssetEditorModule::UploadBuffer(Inventory::InventoryUploadBufferEventData *data)
{
    framework_->GetEventManager()->SendEvent(inventoryEventCategory, Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER, data);
}

bool OgreAssetEditorModule::IsSupportedAssetTypes(const QString &type) const
{
    if (type == "OgreMesh" || type == "OgreMaterial" || type == "OgreParticle" || type == "Audio" || type == "Texture")
        return true;
    else
        return false;
}

void OgreAssetEditorModule::OnContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets)
{
    if (targets.size())
    {
        foreach(QObject *target, targets)
            if (!dynamic_cast<IAsset *>(target))
                return;

        AssetPtr asset = dynamic_cast<IAsset *>(targets[0])->shared_from_this();
        if (IsSupportedAssetTypes(asset->Type()))
        {
            menu->addSeparator();
            EditorAction *openAction = new EditorAction(asset, tr("Open"), menu);
            connect(openAction, SIGNAL(triggered()), SLOT(OpenAssetInEditor()));
            menu->addAction(openAction);
        }
    }
}

void OgreAssetEditorModule::OpenAssetInEditor()
{
    EditorAction *action = dynamic_cast<EditorAction *>(sender());
    if (!action)
        return;
    if (action->asset.expired())
        return;

    AssetPtr asset = action->asset.lock();
    QWidget *editor = 0;
    QString assetName = OgreRenderer::SanitateAssetIdForOgre(asset->Name()).c_str();

    if (asset->Type() == "OgreMesh")
    {
        MeshPreviewEditor *meshEditor = new MeshPreviewEditor(framework_);
        meshEditor->Open(assetName);
        editor = meshEditor;
    }
    else if (asset->Type() == "OgreMaterial")
    {
        OgreScriptEditor *scriptEditor = new OgreScriptEditor(QString(), RexTypes::RexAT_MaterialScript, assetName);
        scriptEditor->Open();
        editor = scriptEditor;
    }
    else if (asset->Type() == "OgreParticle")
    {
        OgreScriptEditor *scriptEditor = new OgreScriptEditor(QString(), RexTypes::RexAT_ParticleScript, assetName);
        editor = scriptEditor;

    }
    /*else if (asset->Type() == "Audio")
    {
        //AudioPreviewEditor *editor = new AudioPreviewEditor(framework_, id, at, name);
    }*/
    else if (asset->Type() == "Texture")
    {
        TexturePreviewEditor *texEditor = new TexturePreviewEditor(0);
        texEditor->OpenOgreTexture(assetName);
        editor = texEditor;
    }

    if (editor)
    {
        editor->setParent(framework_->Ui()->MainWindow());
        editor->setWindowFlags(Qt::Tool);
        editor->show();
    }
}


extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(OgreAssetEditorModule)
POCO_END_MANIFEST
