/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   PropertyTableWidget.h
 *  @brief  Inherits QTableWidget and adds custom drop-functionality.
 */

#pragma once

#include <QTableWidget>

/// Inherits QTableWidget and adds custom drop-functionality.
/// @cond PRIVATE
class PropertyTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    /// Constuctor.
    /** @param parent Parent widget. */
    explicit PropertyTableWidget(QWidget *parent = 0);

    /// Constuctor.
    /** @param rows Number of rows.
        @param columns Number of columns.
        @param parent Parent widget. */
    explicit PropertyTableWidget(int rows, int columns, QWidget *parent = 0);

    /// Destructor.
    ~PropertyTableWidget();

protected:
    /// QTableWidget override.
    void dragMoveEvent(QDragMoveEvent *event);

    /// QTableWidget override.
    QStringList mimeTypes() const;

    /// QTableWidget override.
    bool dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action);

    /// QTableWidget override.
    Qt::DropActions supportedDropActions() const;

private:
    Q_DISABLE_COPY(PropertyTableWidget);

    void InitWidget();
};
/// @endcond
