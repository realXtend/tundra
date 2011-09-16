/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreAssetEditorModule.h
 *  @brief  Provides editing and previewing tools for various asset types.
 */

#pragma once

#include "IModule.h"
#include "AssetFwd.h"
#include "OgreAssetEditorModuleApi.h"

#include <QObject>
#include <QAction>

class QMenu;

/// Utility class that wraps QAction and stores an asset pointer.
class EditorAction : public QAction
{
    Q_OBJECT

public:
    EditorAction(const AssetPtr &assetPtr, const QString &text, QMenu *menu);
    AssetWeakPtr asset;
};

/// Provides editing and previewing tools for various asset types.
class ASSET_EDITOR_MODULE_API OgreAssetEditorModule : public IModule
{
    Q_OBJECT

public:
    OgreAssetEditorModule();
    ~OgreAssetEditorModule();

    void Initialize();
    void Uninitialize();

public slots:
    bool IsSupportedAssetTypes(const QString &type) const;

private:
    Q_DISABLE_COPY(OgreAssetEditorModule);

private slots:
    void OnContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets);

    void OpenAssetInEditor();
};
