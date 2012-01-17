/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
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

/// Provides editing and previewing tools for various asset types.
/** @todo rename this class to AssetEditorModule as it handles more than just Ogre assets. */
class ASSET_EDITOR_MODULE_API OgreAssetEditorModule : public IModule
{
    Q_OBJECT

public:
    OgreAssetEditorModule();
    ~OgreAssetEditorModule();

    void Initialize();
    void Uninitialize();

public slots:
    /// Returns if asset of @type is supported for editing/previewing.
    bool IsSupportedAssetType(const QString &type) const;

private:
    Q_DISABLE_COPY(OgreAssetEditorModule);

private slots:
    void OnContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets);

    void OpenAssetInEditor();
};

/// Utility class that wraps QAction and stores an asset pointer.
/// @cond PRIVATE
class EditorAction : public QAction
{
    Q_OBJECT

public:
    EditorAction(const AssetPtr &assetPtr, const QString &text, QMenu *menu);
    AssetWeakPtr asset;
};
/// @endcond
