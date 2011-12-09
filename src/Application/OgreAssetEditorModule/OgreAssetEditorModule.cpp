/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
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
#include "OgreMaterialEditor.h"

#include "LoggingFunctions.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "IAsset.h"

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
            menu->insertAction(menu->actions().size() > 0 ? menu->actions().first() : 0, openAction);

            int offset = 1;
            // Ogre materials are opened in the visual editor by default.
            // Add another action for opening in raw text editor.
            if (asset->Type() == "OgreMaterial")
            {
                EditorAction *openRawAction = new EditorAction(asset, tr("Open as text"), menu);
                connect(openRawAction, SIGNAL(triggered()), SLOT(OpenAssetInEditor()));
                openRawAction->setObjectName("EditRaw");
                menu->insertAction(*(menu->actions().begin() + offset), openRawAction);
                ++offset;
            }

            menu->insertSeparator(*(menu->actions().begin() + offset));
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
        editor = new MeshPreviewEditor(asset, framework_);
    }
    else if (asset->Type() == "OgreMaterial")
    {
        if (action->objectName() == "EditRaw")
            editor = new OgreScriptEditor(asset, framework_);
        else
            editor = new OgreMaterialEditor(asset, framework_);
    }
    else if (asset->Type() == "OgreParticle")
    {
        editor = new OgreScriptEditor(asset, framework_);
    }
    else if (asset->Type() == "Audio")
    {
        editor = new AudioPreviewEditor(asset, framework_);
    }
    else if (asset->Type() == "Texture")
    {
        editor = new TexturePreviewEditor(asset, framework_);
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
