// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryTreeView.h
 *  @brief  Inventory tree view UI widget.
 */

#ifndef incl_InventoryModule_InventoryTreeView_h
#define incl_InventoryModule_InventoryTreeView_h

#include <QTreeView>

class QWidget;
class QAction;

namespace Inventory
{
    class InventoryWindow;

    class InventoryTreeView : public QTreeView
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param parent Parent widget.
        InventoryTreeView(QWidget *parent = 0);

        /// Destructor.
        virtual ~InventoryTreeView();

    protected:
        /// QAbstractItemView override.
        void contextMenuEvent(QContextMenuEvent *event);

        /// QAbstractItemView override.
        void dragEnterEvent(QDragEnterEvent *event);

        /// QAbstractItemView override.
        void dragMoveEvent(QDragMoveEvent *event);

        /// QAbstractItemView override.
        void dropEvent(QDropEvent *event);
    };
}

#endif
