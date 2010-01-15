// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeView.h
 *  @brief Inventory tree view UI widget.
 */

#ifndef incl_InventoryModule_InventoryTreeView_h
#define incl_InventoryModule_InventoryTreeView_h

#include <QTreeView>

class QWidget;
class QAction;

namespace Inventory
{
    class InventoryTreeView : public QTreeView
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param parent Parent widget.
        InventoryTreeView(QWidget *parent = 0);

        /// Destructor.
        virtual ~InventoryTreeView();

    public slots:
        void Test();

    protected:
        /// QAbstractItemView override.
        void contextMenuEvent(QContextMenuEvent *event);

        /// QAbstractItemView override.
        void dragEnterEvent(QDragEnterEvent *event);

        /// QAbstractItemView override.
        void dragMoveEvent(QDragMoveEvent *event);

        /// QAbstractItemView override.
        void dropEvent(QDropEvent *event);

    private:
        /// Creates the context menu actions.
        void CreateActions();

        /// Action menu.
        QMenu *actionMenu_;

        /// Delete action.
        QAction *actionDelete_;

        /// Rename action.
        QAction *actionRename_;

        /// Cut action.
        QAction *actionCut_;

        /// Paste action.
        QAction *actionPaste_;

        /// New Folder action.
        QAction *actionNewFolder_;

        /// Open action.
        QAction *actionOpen_;

        /// Copy asset UUID action.
        QAction *actionCopyAssetId_;
    };
}

#endif
