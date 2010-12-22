/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetsWindow.h
 *  @brief  
 *
 *          Detailed.
 */

#ifndef incl_SceneStructureModule_AssetsWindow_h
#define incl_SceneStructureModule_AssetsWindow_h

#include "ForwardDefines.h"
#include "AssetFwd.h"

#include <QWidget>

#include <set>

class QTreeWidgetItem;

class AssetTreeWidget;

/// 
/**
*/
class AssetsWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param fw Framework.
        @parent parent Parent widget.
    */
    explicit AssetsWindow(Foundation::Framework *fw, QWidget *parent = 0);

    /// Destructor.
    ~AssetsWindow();

private:
    /// Populates the tree widget with all assets from all asset storages.
    void PopulateTreeWidget();

    /// If @c asset has asset references, adds the asset references as children to the @c parent.
    /** @param asset Asset to be added to the tree widget.
        @param parent The newly created (parent) item.
    */
    void AddChildren(const AssetPtr &asset, QTreeWidgetItem *parent);

    Foundation::Framework *framework; ///< Framework pointer.
    AssetTreeWidget *treeWidget; ///< Tree widget showing the assets.
    QTreeWidgetItem *noProviderItem; ///< "No provider" parent item for assets without storage.'
    std::set<AssetWeakPtr> alreadyAdded; ///< Set of already added assets.

private slots:
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

    /// Expands or collapses the whole tree view, depending on the previous action.
    void ExpandOrCollapseAll();
};

#endif
