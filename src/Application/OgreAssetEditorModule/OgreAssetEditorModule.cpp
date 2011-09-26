/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreAssetEditorModule.cpp
 *  @brief  Provides editing and previewing tools for various asset types.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "OgreAssetEditorModule.h"
#include "OgreScriptEditor.h"
#include "TexturePreviewEditor.h"
#include "AudioPreviewEditor.h"
#include "MeshPreviewEditor.h"

#include "LoggingFunctions.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "IAsset.h"
#include "OgreConversionUtils.h"

#include "MemoryLeakCheck.h"

EditorAction::EditorAction(const AssetPtr &assetPtr, const QString &text, QMenu *menu) :
    QAction(text, menu),
    asset(assetPtr)
{
}

OgreAssetEditorModule::OgreAssetEditorModule() : IModule("OgreAssetEditor")
{
}

OgreAssetEditorModule::~OgreAssetEditorModule()
{
}

void OgreAssetEditorModule::Initialize()
{
    connect(framework_->Ui(), SIGNAL(ContextMenuAboutToOpen(QMenu *, QList<QObject *>)), SLOT(OnContextMenuAboutToOpen(QMenu *, QList<QObject *>)));
}

void OgreAssetEditorModule::Uninitialize()
{
}

bool OgreAssetEditorModule::IsSupportedAssetType(const QString &type) const
{
    if (/*type == "OgreMesh" || */type == "OgreMaterial" || type == "OgreParticle" || type == "Audio" || type == "Texture")
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
        if (IsSupportedAssetType(asset->Type()))
        {
            EditorAction *openAction = new EditorAction(asset, tr("Open"), menu);
            openAction->setObjectName("Edit");
            connect(openAction, SIGNAL(triggered()), SLOT(OpenAssetInEditor()));
            menu->insertAction(menu->actions().first(), openAction);
            menu->insertSeparator(*(menu->actions().begin()+1));
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

    QWidget *editor = 0;
    AssetPtr asset = action->asset.lock();

    if (asset->Type() == "OgreMesh")
    {
        MeshPreviewEditor *meshEditor = new MeshPreviewEditor(asset, framework_);
        editor = meshEditor;
    }
    else if (asset->Type() == "OgreMaterial" || asset->Type() == "OgreParticle")
    {
        OgreScriptEditor *scriptEditor = new OgreScriptEditor(asset, framework_);
        editor = scriptEditor;
    }
    else if (asset->Type() == "Audio")
    {
        AudioPreviewEditor *audioEditor = new AudioPreviewEditor(asset, framework_);
        editor = audioEditor;
    }
    else if (asset->Type() == "Texture")
    {
        TexturePreviewEditor *texEditor = new TexturePreviewEditor(asset, framework_);
        editor = texEditor;
    }

    if (editor)
    {
        editor->setParent(framework_->Ui()->MainWindow());
        editor->setWindowFlags(Qt::Tool);
        editor->setAttribute(Qt::WA_DeleteOnClose);
        editor->show();
    }
}

extern "C"
{
DLLEXPORT void TundraPluginMain(Framework *fw)
{
    IModule *module = new OgreAssetEditorModule();
    fw->RegisterModule(module);
}
}
