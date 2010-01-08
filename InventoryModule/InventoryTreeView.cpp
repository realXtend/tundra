// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeView.cpp
 *  @brief Inventory tree view UI widget.
 */

#include "StableHeaders.h"
#include "InventoryTreeView.h"
#include "AbstractInventoryItem.h"

#include <QWidget>
#include <QDragEnterEvent>
#include <QApplication>

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
}

// virtual
InventoryTreeView::~InventoryTreeView()
{
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
    else
        event->ignore();
}

}
