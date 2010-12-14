/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetTreeWidget.cpp
 *  @brief  Tree widget showing all available assets.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AssetTreeWidget.h"

#include "AssetAPI.h"
#include "IAsset.h"
#include "AssetCache.h"

#include "MemoryLeakCheck.h"

AssetItem::AssetItem(const AssetPtr &asset, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
        assetPtr(asset)
{
    setText(0, asset->Name());
}

AssetPtr AssetItem::Asset() const
{
    return assetPtr.lock();
}

AssetTreeWidget::AssetTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw),
    contextMenu(0)
{
//    setDragDropMode(QAbstractItemView::DropOnly/*DragDrop*/);
//    setDropIndicatorShown(true);
}

AssetTreeWidget::~AssetTreeWidget()
{
}

void AssetTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    // Do mousePressEvent so that the right item gets selected before we show the menu
    // (right-click doesn't do this automatically).
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, e->pos(), e->globalPos(),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    mousePressEvent(&mouseEvent);

    // Create context menu and show it.
    SAFE_DELETE(contextMenu);
    contextMenu = new QMenu(this);

    AddAvailableActions(contextMenu);

    contextMenu->popup(e->globalPos());
}


void AssetTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        foreach(QUrl url, data->urls())
            if (!GetResourceTypeFromResourceFileName(url.path().toStdString().c_str()).isEmpty())
                e->acceptProposedAction();
    }
    else
        QTreeWidget::dropEvent(e);
}

void AssetTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        foreach(QUrl url, data->urls())
            if (!GetResourceTypeFromResourceFileName(url.path().toStdString().c_str()).isEmpty())
                e->acceptProposedAction();
    }
    else
        QTreeWidget::dragMoveEvent(e);
}

void AssetTreeWidget::dropEvent(QDropEvent *e)
{
    const QMimeData *data = e->mimeData();
    if (data->hasUrls())
    {
        QStringList filenames;
        foreach(QUrl url, data->urls())
            if (!GetResourceTypeFromResourceFileName(url.path().toStdString().c_str()).isEmpty())
                filenames << url.path();

        if (!filenames.isEmpty())
        {
            e->acceptProposedAction();
            //Upload(filenames);
        }
    }
    else
        QTreeWidget::dropEvent(e);
}

void AssetTreeWidget::AddAvailableActions(QMenu *menu)
{
    QList<AssetItem *> items = GetSelection();
    if (items.isEmpty())
        return;

    QMenu *deleteMenu = new QMenu(tr("Delete"), menu);
    QAction *deleteSourceAction = new QAction(tr("Delete from source"), deleteMenu);
    deleteSourceAction->setDisabled(true);
    QAction *deleteCacheAction = new QAction(tr("Delete from cache"), deleteMenu);
    QAction *forgetAction = new QAction(tr("Forget asset"), deleteMenu);

    deleteMenu->addAction(deleteSourceAction);
    deleteMenu->addAction(deleteCacheAction);
    deleteMenu->addAction(forgetAction);
    menu->addMenu(deleteMenu);

    connect(deleteSourceAction, SIGNAL(triggered()), SLOT(DeleteFromSource()));
    connect(deleteCacheAction, SIGNAL(triggered()), SLOT(DeleteFromCache()));
    connect(forgetAction, SIGNAL(triggered()), SLOT(Forget()));

    QMenu *reloadMenu = new QMenu(tr("Reload"), menu);
    QAction *reloadFromSourceAction = new QAction(tr("Reload from source"), deleteMenu);
    QAction *reloadFromCacheAction = new QAction(tr("Reload from cache"), deleteMenu);
    QAction *unloadAction = new QAction(tr("Unload"), deleteMenu);

    // Reload from cache is not possible if asset's disk source is empty.
    foreach(AssetItem *item, items)
        if (item->Asset()->DiskSource().trimmed().isEmpty())
        {
            reloadFromCacheAction->setDisabled(true);
            break;
        }

    reloadMenu->addAction(reloadFromSourceAction);
    reloadMenu->addAction(reloadFromCacheAction);
    reloadMenu->addAction(unloadAction);
    menu->addMenu(reloadMenu);

    connect(reloadFromSourceAction, SIGNAL(triggered()), SLOT(ReloadFromSource()));
    connect(reloadFromCacheAction, SIGNAL(triggered()), SLOT(ReloadFromCache()));
    connect(unloadAction, SIGNAL(triggered()), SLOT(Unload()));
/*
    QAction *importAction = new QAction(tr("Import..."), menu);
    QAction *exportAction = new QAction(tr("Export..."), menu);
    menu->addAction(importAction);
    menu->addAction(exportAction);

    connect(importAction, SIGNAL(triggered()), SLOT(Import()));
    connect(exportAction, SIGNAL(triggered()), SLOT(Export()));
*/
}

QList<AssetItem *> AssetTreeWidget::GetSelection() const
{
    QList<AssetItem *> assetItems;
    foreach(QTreeWidgetItem *item, selectedItems())
    {
        // Omit top-level i.e. asset storage items
        AssetItem *aitem = dynamic_cast<AssetItem *>(item);
        if (aitem)
            assetItems << aitem;
    }

    return assetItems;
}

void AssetTreeWidget::DeleteFromSource()
{
/*
    foreach(AssetItem *item, GetSelection())
        framework->Asset()->ForgetAsset(asset, false);

*/
}

void AssetTreeWidget::DeleteFromCache()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
            framework->Asset()->GetAssetCache()->DeleteAsset(item->Asset()->Name());
}

void AssetTreeWidget::Forget()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
            framework->Asset()->ForgetAsset(item->Asset(), false);
}

void AssetTreeWidget::Unload()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
        {
            item->Asset()->Unload();
            QTreeWidgetItem *parent = item->parent();
            parent->removeChild(item);
            SAFE_DELETE(item);
            ///\todo Preferrably use the AssetDeleted() signal from AssetAPI for deleting items.
        }
}

void AssetTreeWidget::ReloadFromCache()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
            item->Asset()->LoadFromCache();
}

void AssetTreeWidget::ReloadFromSource()
{
    foreach(AssetItem *item, GetSelection())
        if (item->Asset())
        {
            framework->Asset()->ForgetAsset(item->Asset(), false);
            framework->Asset()->RequestAsset(item->Asset()->Name());
        }
}

void AssetTreeWidget::Import()
{
}

void AssetTreeWidget::Export()
{
}

void AssetTreeWidget::Upload()
{
}
