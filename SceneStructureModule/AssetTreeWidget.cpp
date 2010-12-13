/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AssetTreeWidget.cpp
 *  @brief  Tree widget showing all available assets.
 */

#include "StableHeaders.h"
#include "AssetTreeWidget.h"

AssetTreeWidget::AssetTreeWidget(Foundation::Framework *fw, QWidget *parent) :
    QTreeWidget(parent),
    framework(fw),
    contextMenu(0)
{
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

//    AddAvailableActions(contextMenu);

    contextMenu->popup(e->globalPos());
}

/*
void AssetTreeWidget::dragEnterEvent(QDragEnterEvent *e)
{
}

void AssetTreeWidget::dragMoveEvent(QDragMoveEvent *e)
{
}

void AssetTreeWidget::dropEvent(QDropEvent *e)
{
}
*/

void AssetTreeWidget::AddAvailableActions(QMenu *menu)
{
}
