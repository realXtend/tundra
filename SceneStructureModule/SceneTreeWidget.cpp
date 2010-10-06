/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeView.cpp
 *  @brief  Tree widget showing the scene structure.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "SceneTreeWidget.h"

#include "ECEditorWindow.h"
#include "UiServiceInterface.h"

//#include <QWidget>
//#include <QDragEnterEvent>
//#include <QUrl>
//#include <QMenu>

#include "MemoryLeakCheck.h"

SceneTreeWidget::SceneTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw)
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
    connect(this, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(Edit()));
}

SceneTreeWidget::~SceneTreeWidget()
{
}

void SceneTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    // Do mousePressEvent so that the right item gets selected before we show the menu
    // (right-click doesn't do this automatically).
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, e->pos(), e->globalPos(),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    mousePressEvent(&mouseEvent);

    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    // Create context menu
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // Create context menu actions, connect them to slots and add to menu
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *editAction = new QAction(tr("Edit"), menu);
    QAction *editInNewAction = new QAction(tr("Edit in new window"), menu);
/*
    QAction *renameAction = new QAction(tr("Rename"), menu);
    QAction *deleteAction = new QAction(tr("Delete"), menu);
    QAction *copyAction = new QAction(tr("Copy"), menu);
    QAction *pasteAction = new QAction(tr("Paste"), menu);
*/
    connect(editAction, SIGNAL(triggered()), SLOT(Edit()));
    connect(editInNewAction, SIGNAL(triggered()), SLOT(EditInNew()));
/*
    connect(renameAction, SIGNAL(triggered()), SLOT(Rename()));
    connect(deleteAction, SIGNAL(triggered()), SLOT(Delete));
    connect(copyAction, SIGNAL(triggered()), SLOT(Copy));
    connect(pasteAction, SIGNAL(triggered()), SLOT(Paste));
*/
    menu->addAction(editAction);
    menu->addAction(editInNewAction);
/*
    menu->addAction(renameAction);
    menu->addAction(deleteAction);
    menu->addAction(copyAction);
    menu->addAction(pasteAction);
*/
    // Show menu.
    menu->popup(e->globalPos());
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

void SceneTreeWidget::Edit()
{
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    assert(ui);

    SceneTreeWidgetItem *item = static_cast<SceneTreeWidgetItem *>(topLevelItem(index.row()));

    // If we have existing editor instance, use it.
    if (ecEditor)
    {
        ecEditor->AddEntity(item->id);
        ecEditor->RefreshPropertyBrowser();
        ui->BringWidgetToFront(ecEditor);
        return;
    }

    // Create new instance
    ecEditor = new ECEditor::ECEditorWindow(framework);
    ecEditor->setAttribute(Qt::WA_DeleteOnClose);
    ecEditor->move(this->pos().x() + 100, this->pos().y() + 100);
    ecEditor->hide();
    ecEditor->AddEntity(item->id);
    ecEditor->RefreshPropertyBrowser();

    ui->AddWidgetToScene(ecEditor);
    ui->ShowWidget(ecEditor);
    ui->BringWidgetToFront(ecEditor);
}

void SceneTreeWidget::EditInNew()
{
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    // Create new instance every time.
    UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
    assert(ui);

    SceneTreeWidgetItem *item = static_cast<SceneTreeWidgetItem *>(topLevelItem(index.row()));

    ECEditor::ECEditorWindow *editor = new ECEditor::ECEditorWindow(framework);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->move(this->pos().x() + 100, this->pos().y() + 100);
    editor->hide();
    editor->AddEntity(item->id);
    editor->RefreshPropertyBrowser();

    ui->AddWidgetToScene(editor);
    ui->ShowWidget(editor);
    ui->BringWidgetToFront(editor);
}

void SceneTreeWidget::Rename()
{
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

}

void SceneTreeWidget::Delete()
{
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

}

void SceneTreeWidget::Copy()
{
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

}

void SceneTreeWidget::Paste()
{
    QModelIndex index = selectionModel()->currentIndex();
    if (!index.isValid())
        return;

}
