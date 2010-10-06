/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeView.cpp
 *  @brief  Tree widget showing the scene structure.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SceneTreeWidget.h"

//#include <QWidget>
//#include <QDragEnterEvent>
//#include <QUrl>
//#include <QMenu>

#include "MemoryLeakCheck.h"

SceneTreeWidget::SceneTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers/*EditKeyPressed*/);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    //setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setHeaderHidden(true);
/*
    setStyleSheet(
        "QTreeView::branch:has-siblings:!adjoins-item"
        "{"
            "border-image: url(:/images/iconBranchVLine.png) 0;"
        "}"
        "QTreeView::branch:has-siblings:adjoins-item"
        "{"
            "border-image: url(:/images/iconBranchMore.png) 0;"
        "}"
        "QTreeView::branch:!has-children:!has-siblings:adjoins-item"
        "{"
            "border-image: url(:/images/iconBranchEnd.png) 0;"
        "}"
        "QTreeView::branch:has-children:!has-siblings:closed,"
        "QTreeView::branch:closed:has-children:has-siblings"
        "{"
            "border-image: none;"
            "image: url(:/images/iconBranchClosed.png);"
        "}"
        "QTreeView::branch:open:has-children:!has-siblings,"
        "QTreeView::branch:open:has-children:has-siblings"
        "{"
            "border-image: none;"
            "image: url(:/images/iconBranchOpen.png);"
        "}");
*/
}

SceneTreeWidget::~SceneTreeWidget()
{
}

void SceneTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
/*
    // Do mousePressEvent so that the right item gets selected before we show the menu
    // (right-click doesn't do this automatically).
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, e->pos(), e->globalPos(),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    mousePressEvent(&mouseEvent);

    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *editXml = new QAction(tr("Edit XML..."), menu);
    QAction *deleteEntity= new QAction(tr("Delete"), menu);
    QAction *addComponent = new QAction(tr("Add new component..."), menu);
    QAction *copyEntity = new QAction(tr("Copy"), menu);
    QAction *pasteEntity = new QAction(tr("Paste"), menu);

    menu->addAction(editXml);
    menu->addAction(deleteEntity);
    menu->addAction(addComponent);
    menu->addAction(copyEntity);
    menu->addAction(pasteEntity);
*/
/*
    QListIterator<QAction *> it(actions());
    while(it.hasNext())
    {
        QAction *action = it.next();
        if (action->isEnabled())
        {
            // This is kind of hack, but we might have case that base language is not english.
            InventoryAction* act = qobject_cast<InventoryAction* >(action);
            QString text = QApplication::translate("Inventory::InventoryWindow", act->GetText().toStdString().c_str());
            action->setText(text);

            menu->addAction(action);
        }
   }
    if (menu->actions().size() > 1) // separator "action" is always enabled, hence the 1
        menu->popup(event->globalPos());
*/
//    menu->popup(e->globalPos());
}

void SceneTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
/*
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
*/
}

void SceneTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
/*
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        if (event->source() == this)
        {
            InventoryItemModel *itemModel = checked_static_cast<InventoryItemModel *>(model());
            assert(itemModel);

            AbstractInventoryItem *draggedItem = itemModel->GetItem(selectionModel()->currentIndex());
            assert(draggedItem);
            QModelIndex destIndex = indexAt(event->pos());

            if (!destIndex.isValid())
            {
                event->ignore();
                return;
            }

            AbstractInventoryItem *destItem = 0;

            if (itemModel->GetItem(destIndex)->GetItemType() == AbstractInventoryItem::Type_Asset)
                destItem = itemModel->GetItem(destIndex)->GetParent();
            else
                destItem = itemModel->GetItem(destIndex);

            if (!destItem || destItem->IsLibraryItem() || (draggedItem->GetParent() == destItem))
            {
                event->ignore();
                return;
            }

            if (draggedItem ->IsLibraryItem())
                event->setDropAction(Qt::CopyAction);
            else
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
*/
}

void SceneTreeWidget::dropEvent(QDropEvent *e)
{
/*
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
            filenames << it.next().path();

        if (!filenames.isEmpty())
            m->UploadFiles(filenames, itemnames, 0);

        event->acceptProposedAction();

        itemModel->CheckTreeForDirtys();
    }
    else
        QTreeView::dropEvent(event);
*/
}

