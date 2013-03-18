/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   AssetItemMenuHandler.h
    @brief  Handles populating of assets and asset storages context menus and their chosen actions. */

#pragma once

#include "SceneFwd.h"
#include "AssetFwd.h"

#include <QObject>

class QMenu;

struct AssetAndStorageItems
{
    QList<IAsset*> assets;
    QList<IAssetStorage*> storages;
};

class AssetItemMenuHandler : public QObject
{
    Q_OBJECT
public:
    AssetItemMenuHandler(Framework *fw);

private slots:
    void AddAssetMenuItems(QMenu * menu, QList<QObject *> targets);
    
    /// Opens Request New Asset dialog.
    void RequestNewAsset();

    /// Creates new programmatic asset.
    void CreateAsset();

    /// Opens the location folder of an asset in operating system's file explorer.
    void OpenFileLocation();

    /// Opens the asset in an external editor, as reported by the operating system.
    void OpenInExternalEditor();

    /// Clones selected asset(s).
    void Clone();

    /// Imports new asset(s).
    void Import();

    /// Exports selected asset(s).
    void Export();

    /// Copies asset reference to clipboard.
    void CopyAssetRef();

    /// Reload selected asset(s) from source.
    void ReloadFromSource();

    /// Reload selected asset(s) from cache.
    void ReloadFromCache();

    /// Unloads selected asset(s).
    void Unload();

    /// Forgets selected asset(s).
    void Forget();

    /// Deletes selected asset(s) from source.
    void DeleteFromSource();
    
    /// Deletes selected assets(s) from cache.
    void DeleteFromCache();

    /// Shows dialog for invoking functions for currently selected assets.
    void OpenFunctionDialog();

    /// Makes the currently selected asset storage the default system storage.
    void MakeDefaultStorage();

    /// Removes the currently selected asset storages from the system.
    void RemoveStorage();

    /// Called by Request New Asset dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0. */
    void RequestNewAssetDialogClosed(int result);

    /// Called by CloneAssetDialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0. */
    void CloneAssetDialogClosed(int result);

    /// Called by open file dialog when it's closed.
    /** @param result Result of dialog closure. Open is 1, Cancel is 0. */
    void OpenFileDialogClosed(int result);

    /// Called by SaveAssetAs save file dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0. */
    void SaveAssetDialogClosed(int result);

    /// Called by function dialog when it's finished.
    /** @param result Result of finished. Close is 0, Execute and Close is 1, Execute is 2. */
    void FunctionDialogFinished(int result);
private:
    Framework *framework_;
    AssetAndStorageItems targets_;
    QObject *sender_;
};
