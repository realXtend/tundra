/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryTreeView.h
 *  @brief  Inventory tree view UI widget.
 */

#ifndef incl_InventoryModule_InventoryTreeView_h
#define incl_InventoryModule_InventoryTreeView_h

#include <QTreeView>
#include <QAction>

class QWidget;

namespace Inventory
{
    class InventoryWindow;

    /// This is helper class for translations.
    class InventoryAction : public QAction 
    {
        Q_OBJECT 

     public:
        InventoryAction(QWidget *parent = 0) : QAction(parent) {}
        InventoryAction(const QString& text, QWidget *parent = 0) : QAction(text, parent), orginal_text(text) {}
        QString GetText() const { return orginal_text; }
     private:
        QString orginal_text;
    };

    /// Inventory tree view UI widget.
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
