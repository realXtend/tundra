/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetTreeWidget.h
 *  @brief  Tree widget showing all available assets.
 */

#ifndef incl_SceneStructureModule_AssetTreeWidget_h
#define incl_SceneStructureModule_AssetTreeWidget_h

#include <QTreeWidget>

#include "ForwardDefines.h"
#include "AssetFwd.h"

class QMenu;

/// Item representing asset in the tree widget.
class AssetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param asset Asset pointer.
        @param parent Parent tree widget item.
    */
    AssetItem(const AssetPtr &asset, QTreeWidgetItem *parent = 0);

    /// Returns shared pointer to the asset. Always remember to check that the pointer is not null.
    AssetPtr Asset() const;

private:
    AssetWeakPtr assetPtr; ///< Weak pointer to the asset.
};

/// Tree widget showing all available assets.
class AssetTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param fw Framework pointer.
        @param parent Parent widget.
    */
    AssetTreeWidget(Foundation::Framework *fw, QWidget *parent = 0);

    /// Destructor.
    virtual ~AssetTreeWidget();

protected:
    /// QWidget override. Show context menu with asset-spesific actions.
    void contextMenuEvent(QContextMenuEvent *e);

    /// QWidget override.
    void dragEnterEvent(QDragEnterEvent *e);

    /// QWidget override.
    void dragMoveEvent(QDragMoveEvent *e);

    /// QWidget override.
    void dropEvent(QDropEvent *e);

private:
    /// Creates and adds applicable actions to the right-click context menu.
    /** @param menu Context menu.
    */
    void AddAvailableActions(QMenu *menu);

    /// Returns list of selected asset items.
    QList<AssetItem *> GetSelection() const;

    Foundation::Framework *framework; ///< Framework.
    QMenu *contextMenu; ///< Right-click context menu.

private slots:
    /// Deletes selected asset(s) from source.
    void DeleteFromSource();

    /// Deletes selected assets(s) from cache.
    void DeleteFromCache();

    /// Forgets selected asset(s).
    void Forget();

    /// Unloads selected asset(s).
    void Unload();

    /// Reload selected asset(s) from cache.
    void ReloadFromCache();

    /// Reload selected asset(s) from source.
    void ReloadFromSource();

    /// Imports new asset(s).
    void Import();

    /// Called by open file dialog when it's closed.
    /** @param result Result of dialog closure. Open is 1, Cancel is 0.
    */
    void OpenFileDialogClosed(int result);

    /// 
    void RequestNewAsset();

    /// Exports selected asset(s).
    void Export();

    /// Called by SaveAssetAs save file dialog when it's closed.
    /** @param result Result of dialog closure. Save is 1, Cancel is 0.
    */
    void SaveAssetDialogClosed(int result);

    ///
    /** @param files List of filenames.
    */
    void Upload(const QStringList &files);

    /// Opens the location folder of an asset in operating system's file explorer.
    void OpenFileLocation();
};

#endif
