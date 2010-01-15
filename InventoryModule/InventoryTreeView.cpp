// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeView.cpp
 *  @brief Inventory tree view UI widget.
 */

#include "StableHeaders.h"
#include "InventoryTreeView.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"

#include <QWidget>
#include <QDragEnterEvent>
#include <QUrl>
#include <QMenu>

namespace Inventory
{

InventoryTreeView::InventoryTreeView(QWidget *parent) : QTreeView(parent)
{
    setEditTriggers(QAbstractItemView::EditKeyPressed);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    //CreateActions();
}

// virtual
InventoryTreeView::~InventoryTreeView()
{
}

void InventoryTreeView::contextMenuEvent(QContextMenuEvent *event)
{
/*
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    //model()->flags(index);
    QMenu *menu = new QMenu(this);
    menu->addAction(actionDelete_);
    menu->addAction(actionRename_);
    menu->addAction(actionCut_);
    menu->addAction(actionPaste_);
    menu->addAction(actionNewFolder_);
    menu->addAction(actionOpen_);
    menu->addAction(actionCopyAssetId_);
    menu->popup(event->globalPos());
*/
}

void InventoryTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else if(event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
}

void InventoryTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        if (event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else if(event->mimeData()->hasUrls())
        event->accept();
    else
        event->ignore();
}

void InventoryTreeView::dropEvent(QDropEvent *event)
{
    const QMimeData *data = event->mimeData();
    if (data->hasUrls())
    {
        InventoryItemModel *itemModel = dynamic_cast<InventoryItemModel *>(model());
        if (!itemModel)
        {
            event->ignore();
            return;
        }

        AbstractInventoryDataModel *m = itemModel->GetInventory();
        if (!m)
        {
            event->ignore();
            return;
        }

        QStringList filenames, itemnames;
        QListIterator<QUrl> it(data->urls());
        while(it.hasNext())
            filenames << it.next().path().remove(0, 1); // remove '/' from the beginning

        if (!filenames.isEmpty())
            m->UploadFiles(filenames, itemnames, 0);
    }
    else
        QTreeView::dropEvent(event);
}

void InventoryTreeView::Test()
{
    std::cout << "test" << std::endl;
}

void InventoryTreeView::CreateActions()
{
    actionDelete_ = new QAction(tr("&Delete"), this);
    actionDelete_->setShortcuts(QKeySequence::Delete);
    actionDelete_->setStatusTip(tr("Delete this item"));
    connect(actionDelete_, SIGNAL(triggered()), this, SLOT(Test()));

    actionRename_ = new QAction(tr("&Rename"), this);
    //actionDelete_->setShortcuts();
    actionDelete_->setStatusTip(tr("Rename this item"));
    connect(actionRename_, SIGNAL(triggered()), this, SLOT(Test()));

    actionCut_ = new QAction(tr("&Cut"), this);
    actionDelete_->setShortcuts(QKeySequence::Cut);
    actionDelete_->setStatusTip(tr("Cut this item"));
    connect(actionCut_, SIGNAL(triggered()), this, SLOT(Test()));

    actionPaste_ = new QAction(tr("&Patse"), this);
    actionDelete_->setShortcuts(QKeySequence::Paste);
    actionDelete_->setStatusTip(tr("Paste this item"));
    connect(actionPaste_, SIGNAL(triggered()), this, SLOT(Test()));

    actionNewFolder_ = new QAction(tr("&New Folder"), this);
    actionDelete_->setShortcuts(QKeySequence::Delete);
    actionDelete_->setStatusTip(tr("Create new folder"));
    connect(actionNewFolder_, SIGNAL(triggered()), this, SLOT(Test()));

    actionOpen_ = new QAction(tr("&Open"), this);
    actionDelete_->setShortcuts(QKeySequence::Delete);
    actionDelete_->setStatusTip(tr("Open this item"));
    connect(actionOpen_, SIGNAL(triggered()), this, SLOT(Test()));

    actionCopyAssetId_ = new QAction(tr("&Copy asset reference"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionDelete_->setStatusTip(tr("Delete this item"));
    connect(actionCopyAssetId_, SIGNAL(triggered()), this, SLOT(Test()));
}

}
