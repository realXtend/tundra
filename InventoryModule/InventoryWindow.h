// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryWindow.h
 *  @brief The inventory window.
 */

#ifndef incl_InventoryModule_InventoryWindow_h
#define incl_InventoryModule_InventoryWindow_h

#include "Foundation.h"

#include <QtGui>
#include <QObject>
#include <QModelIndex>

class QWidget;
class QPushButton;
class QTreeWidgetItem;
class QTreeView;

namespace QtUI
{
    class QtModule;
    class UICanvas;
}

namespace RexLogic
{
    class RexLogicModule;
}

namespace Inventory
{
    class InventoryItemEventData;
    class InventoryItemModel;

    class InventoryWindow : public QObject
    {
        Q_OBJECT

        MODULE_LOGGING_FUNCTIONS

        //! Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { const std::string &name = "InventoryModule::UI"; return name; }

    public:
        /// Constructor.
        /// @param framework Framework.pointer.
        /// @param module RexLogicModule pointer.
        InventoryWindow(Foundation::Framework *framework, RexLogic::RexLogicModule *rexLogic);

        /// Destructor.
        virtual ~InventoryWindow();

    public slots:
        /// Initializes the OpenSim inventory data/view model.
        void InitOpenSimInventoryTreeModel();

        /// Resets the inventory tree model.
        void ResetInventoryTreeModel();

        /// Shows/toggles the inventory window.
        void Toggle();

        /// Hides the inventory window.
        void Hide();

        /// Updates possible actions depending on the currently active tree view item.
        void UpdateActions();

        ///
        void HandleInventoryDescendent(InventoryItemEventData *item_data);

    private slots:
        /// Close handler
        void CloseInventoryWindow();

        /// Fetchs the inventory folder's descendents.
        void FetchInventoryDescendents(const QModelIndex &index);

        /// Adds new folder.
        void AddFolder();

        /// Deletes item (folder/asset).
        void DeleteItem();

        /// Deletes item (folder/asset).
        void RenameItem();

        /// File upload.
        void Upload();

    private:
        Q_DISABLE_COPY(InventoryWindow);

        /// Initializes the inventory UI.
        void InitInventoryWindow();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// RexLogicModule pointer.
        RexLogic::RexLogicModule* rexLogicModule_;

        /// QtModule pointer.
        boost::shared_ptr<QtUI::QtModule> qtModule_;

        /// Inventory window widget.
        QWidget *inventoryWidget_;

        /// Canvas for the inventory window.
        boost::shared_ptr<QtUI::UICanvas> canvas_;

        /// Inventory view model.
        InventoryItemModel *inventoryItemModel_;

        // QWidgets
        QTreeView *treeView_;

        QPushButton *buttonClose_, *buttonDownload_, *buttonUpload_,
            *buttonAddFolder_, *buttonDeleteItem_, *buttonRename_;
    };
}

#endif
