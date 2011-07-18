/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetsWindow.h
 *  @brief  The main UI for managing asset storages and assets.
 */

#pragma once

#include "AssetFwd.h"

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

#include <set>

class QTreeWidgetItem;
class Framework;
class AssetTreeWidget;

/// The main UI for managing asset storages and assets.
/** Most of the functionality provided by AssetsWindow is implemented in AssetTreeWidget.
*/
class AssetsWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param fw Framework.
        @parent parent Parent widget.
    */
    AssetsWindow(Framework *fw, QWidget *parent = 0);

    /// Constructs the window to view only assets of specific type.
    /** @param assetType Asset type identifier, see AssetAPI::GetResourceTypeFromAssetRef() and AssetAPI::GetResourceTypeFromAssetRef()
        @param fw Framework.
        @parent parent Parent widget.
    */
    AssetsWindow(const QString &assetType, Framework *fw, QWidget *parent = 0);

    /// Destructor.
    ~AssetsWindow();

public slots:
    /// Populates the tree widget with all assets from all asset storages.
    void PopulateTreeWidget();

        /// Adds new asset to the tree widget.
    /** @param asset New asset.
    */
    void AddAsset(AssetPtr asset);

    /// Removes asset from the tree widget.
    /** @param asset Asset to be removed.
    */
    void RemoveAsset(AssetPtr asset);

    /// Searches for items containing @c filter (case-insensitive) and toggles their visibility.
    /** If match is found the item is set visible and expanded, otherwise it's hidden.
        @param filter Text used as a filter.
    */
    void Search(const QString &filter);

signals:
    /// 
    /** @param 
    */
    void AssetPicked(AssetPtr asset);

    ///
    void PickCanceled();

private:
    /// Initializes the UI.
    void Initialize();

    /// Event filter to catch and react to child widget events
    virtual bool eventFilter(QObject *obj, QEvent *e);

    /// If @c asset has asset references, adds the asset references as children to the @c parent.
    /** @param asset Asset to be added to the tree widget.
        @param parent The newly created (parent) item.
    */
    void AddChildren(const AssetPtr &asset, QTreeWidgetItem *parent);

    Framework *framework; ///< Framework pointer.
    AssetTreeWidget *treeWidget; ///< Tree widget showing the assets.
    QTreeWidgetItem *noProviderItem; ///< "No provider" parent item for assets without storage.
    std::set<AssetWeakPtr> alreadyAdded; ///< Set of already added assets.
    QLineEdit *searchField; ///< Search field line edit.
    QPushButton *expandAndCollapseButton; ///< Expand/collapse all button.
    QString assetType;

private slots:
    /// Expands or collapses the whole tree view, depending on the previous action.
    void ExpandOrCollapseAll();

    /// Checks the expand status to mark it to the expand/collapse button
    void CheckTreeExpandStatus(QTreeWidgetItem *item);

    /// Unmarks unloaded assets in the UI.
    /** @param asset Asset which was loaded.
    */
    void HandleAssetLoaded(AssetPtr asset);

    /// Marks unloaded assets in the UI.
    /** @param asset Asset which was unloaded.
    */
    void HandleAssetUnloaded(IAsset *asset);

    void PickAsset(QTreeWidgetItem *);
    void PickAssetAndClose();
    void Cancel();
};

