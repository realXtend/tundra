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
#include "ModuleManager.h"
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
/*
    materialWizard = new MaterialWizard;
    connect(materialWizard, SIGNAL(NewMaterial(Inventory::InventoryUploadEventData *)),
        this, SLOT(UploadFile(Inventory::InventoryUploadEventData *)));

    uiService_ = framework_->GetServiceManager()->GetService<UiServiceInterface>(Service::ST_Gui);
    if (!uiService_.expired())
    {
        UiProxyWidget *proxy  = uiService_.lock()->AddWidgetToScene(materialWizard_);
        uiService_.lock()->AddWidgetToMenu(materialWizard_, tr("Material Wizard"), tr("World Tools"),
            Application::InstallationDirectory + "data/ui/images/menus/edbutton_MATWIZ_normal.png");
        connect(proxy, SIGNAL(Closed()), materialWizard_, SLOT(Close()));
    }

    editorManager = new EditorManager;
*/

    connect(framework_->Ui(), SIGNAL(ContextMenuAboutToOpen(QMenu *, QList<QObject *>)), SLOT(OnContextMenuAboutToOpen(QMenu *, QList<QObject *>)));
}

void OgreAssetEditorModule::Uninitialize()
{
    SAFE_DELETE(materialWizard);
//    SAFE_DELETE(editorManager);
}

void OgreAssetEditorModule::Update(f64 frametime)
{
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
            openAction->setObjectName("Edit");
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
        LogInfo("asset->Type() == OgreMaterial");
        OgreScriptEditor *scriptEditor = new OgreScriptEditor(OgreScriptEditor::MaterialScript, assetName);
        scriptEditor->Open();
        editor = scriptEditor;
    }
    else if (asset->Type() == "OgreParticle")
    {
        OgreScriptEditor *scriptEditor = new OgreScriptEditor(OgreScriptEditor::ParticleScript, assetName);
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

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    IModule *module = new OgreAssetEditorModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
