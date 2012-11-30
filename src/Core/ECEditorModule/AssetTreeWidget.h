/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   AssetTreeWidget.h
 *  @brief  Tree widget showing all available assets.
 */

#pragma once

#include <QTreeWidget>

#include "AssetFwd.h"

class QMenu;
class Framework;

class AssetItem;
class AssetStorageItem;
struct AssetTreeWidgetSelection;

/// Tree widget showing all available assets.
class AssetTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param fw Framework pointer.
        @param parent Parent widget. */
    AssetTreeWidget(Framework *fw, QWidget *parent = 0);

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
    /** @param menu Context menu. */
    void AddAvailableActions(QMenu *menu);

    /// Returns list of selected asset and storage items.
    AssetTreeWidgetSelection SelectedItems() const;

    Framework *framework; ///< Framework.
    QMenu *contextMenu; ///< Right-click context menu.

private slots:
    /// Opens Add Content window with the provided @files added to it.
    /** @param files List of filenames. */
    void Upload(const QStringList &files);
};
