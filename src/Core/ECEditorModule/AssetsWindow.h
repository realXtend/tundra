/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   AssetsWindow.h
    @brief  The main UI for managing asset storages and assets. */

#pragma once

#include "AssetFwd.h"
#include "ECEditorModuleApi.h"

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

#include <set>

class QTreeWidgetItem;
class Framework;
class AssetTreeWidget;

/// The main UI for managing asset storages and assets.
/** Assets window can be used either for generic browsing and maintaining of all known
    assets in the system, or as an asset picker/selection tool.
    Most of the functionality provided by AssetsWindow is implemented in AssetTreeWidget. */
class ECEDITOR_MODULE_API AssetsWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param fw Framework.
        @parent parent Parent widget. */
    AssetsWindow(Framework *fw, QWidget *parent = 0);

    /// Constructs the window to view only assets of specific type.
    /** @param assetType Asset type identifier, see AssetAPI::GetResourceTypeFromAssetRef().
        @param fw Framework.
        @parent parent Parent widget. */
    AssetsWindow(const QString &assetType, Framework *fw, QWidget *parent = 0);

    ~AssetsWindow();

public slots:
    /// Populates the tree widget with all assets from all asset storages.
    void PopulateTreeWidget();

    /// Adds new asset to the tree widget.
    /** @param asset New asset. */
    void AddAsset(AssetPtr asset);

    /// Removes asset from the tree widget.
    /** @param asset Asset to be removed. */
    void RemoveAsset(AssetPtr asset);

    /// Searches for items containing @c filter (case-insensitive) and toggles their visibility.
    /** If match is found the item is set visible and expanded, otherwise it's hidden.
        @param filter Text used as a filter. */
    void Search(const QString &filter);

    /// Updates asset item's text and appearance accordingly to the asset properties.
    void UpdateAssetItem(IAsset *asset);
    void UpdateAssetItem(AssetPtr asset) { UpdateAssetItem(asset.get()); }

signals:
    /// Emitted when an asset is selected from the list, can be used for e.g. previewing the asset.
    /** @param asset Asset that is selected. */
    void SelectedAssetChanged(AssetPtr asset);

    /// Emitted when asset was picked (not just selected).
    /** @param asset Asset that was picked. */
    void AssetPicked(AssetPtr asset);

    /// Emitted when asset picking was canceled.
    void PickCanceled();

private:
    /// Initializes the UI.
    void Initialize();

    /// Event filter to catch and react to child widget events
    virtual bool eventFilter(QObject *obj, QEvent *e);

    /// If @c asset has asset references, adds the asset references as children to the @c parent.
    /** @param asset Asset to be added to the tree widget.
        @param parent The newly created (parent) item. */
    void AddChildren(const AssetPtr &asset, QTreeWidgetItem *parent);


    /// As C++ standard weak_ptr doesn't provide less than operator (or any comparison operators for that matter), we need to provide it ourselves.
    struct AssetWeakPtrLessThan
    {
        bool operator() (const AssetWeakPtr &a, const AssetWeakPtr &b) const { return WEAK_PTR_LESS_THAN(a, b); }
    };

    Framework *framework;
    AssetTreeWidget *treeWidget; ///< Tree widget showing the assets.
    QTreeWidgetItem *noProviderItem; ///< "No provider" parent item for assets without storage.
    std::set<AssetWeakPtr, AssetWeakPtrLessThan> alreadyAdded; ///< Set of already added assets.
    QLineEdit *searchField;
    QPushButton *expandAndCollapseButton;
    QString assetType;

private slots:
    /// Expands or collapses the whole tree view, depending on the previous action.
    void ExpandOrCollapseAll();

    /// Checks the expand status to mark it to the expand/collapse button
    void CheckTreeExpandStatus(QTreeWidgetItem *item);

    void AssetDoubleClicked(QTreeWidgetItem *item, int column);
    void ChangeSelectedAsset(QTreeWidgetItem *);
    void PickAssetAndClose();
    void Cancel();
};
