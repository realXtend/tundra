// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   PropertyTableWidget.cpp
 *  @brief  PropertyTableWidget inherits QTableWidget and add some custom drop-functionality.
 */

#include "StableHeaders.h"
#include "PropertyTableWidget.h"

#include <RexUUID.h>

#include <QHeaderView>
#include <QMimeData>
#include <QDragEnterEvent>

namespace Naali
{

PropertyTableWidget::PropertyTableWidget(QWidget *parent) : QTableWidget(parent)
{
    InitWidget();
}

PropertyTableWidget::PropertyTableWidget(int rows, int columns, QWidget *parent) : QTableWidget(rows, columns, parent)
{
    InitWidget();
}

PropertyTableWidget::~PropertyTableWidget()
{
}

void PropertyTableWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
    {
        QModelIndex index  = indexAt(event->pos());
        if (index.isValid())
        {
            QTableWidgetItem * item = itemFromIndex(index);
            if (item && item->flags() & Qt::ItemIsDropEnabled)
            {
                event->accept();
                return;
            }
        }
    }

    event->ignore();
}

QStringList PropertyTableWidget::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.inventory.item";
    return types;
}

bool PropertyTableWidget::dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/vnd.inventory.item"))
        return false;

    QByteArray encodedData = data->data("application/vnd.inventory.item");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    QString asset_id;
    bool valid = false;
    while(!stream.atEnd())
    {
        QString mimedata, asset_type, item_id, name;
        stream >> mimedata;

        QStringList list = mimedata.split(";", QString::SkipEmptyParts);
        if (list.size() < 4)
            continue;

        asset_type = list.at(0);
        if (asset_type.toInt(&valid) != RexTypes::RexAT_Texture)
            continue;

        item_id = list.at(1);
        if (!RexUUID::IsValid(item_id.toStdString()))
            continue;

        name = list.at(2);
        asset_id = list.at(3);
    }

    if (!valid || !RexUUID::IsValid(asset_id.toStdString()))
        return false;

    QTableWidgetItem *item = this->item(row, column);
    if (item && item->flags() & Qt::ItemIsDropEnabled)
        item->setData(Qt::DisplayRole, asset_id);
    else
        return false;

    return true;
}

Qt::DropActions PropertyTableWidget::supportedDropActions() const
{
    return Qt::CopyAction;
}

void PropertyTableWidget::InitWidget()
{
    // Set up drop functionality.
    setAcceptDrops(true);
    setDragEnabled(false);
    setDragDropMode(QAbstractItemView::DropOnly);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropOverwriteMode(true);

    // Set up headers and size.
    setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Type") << tr("Value"));
    verticalHeader()->setVisible(false);
    resizeColumnToContents(0);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

}

