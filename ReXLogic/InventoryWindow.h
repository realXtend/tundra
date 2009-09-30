// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryWindow.h
 *  @brief The inventory window.
 */

#ifndef incl_InventoryWindow_h
#define incl_InventoryWindow_h

#include "Foundation.h"

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

namespace OpenSimProtocol
{
    class InventoryModel;
}

namespace RexLogic
{
    class RexLogicModule;

    class InventoryWindow : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Framework.pointer.
        /// @param module RexLogicModule pointer.
        InventoryWindow(Foundation::Framework *framework, RexLogicModule *rexlogic);

        /// Destructor.
        virtual ~InventoryWindow();

    public slots:
        /// Initializes the inventory data/view model.
        void InitInventoryTreeView();

        void ResetInventoryTreeView();

        /// Shows the inventory window.
        void Show();

        /// Hides the inventory window.
        void Hide();

        /// Updates menu actions.
        void UpdateActions();

    private slots:
        /// Fetchs the inventory folder's descendents.
        void FetchInventoryDescendents(const QModelIndex &index);

        /// Creates new folder.
        void CreateFolder();

        /// Deletes folder.
        void DeleteFolder();

        /// Creates new inventory asset.
        void CreateAsset();

        /// Deletes inventory asset.
        void DeleteAsset();

        /// Informs the server about change in name of folder or asset.
        void NameChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    private:
        Q_DISABLE_COPY(InventoryWindow);

        /// Initializes the inventory UI.
        void InitInventoryWindow();

        /// Pointer to the framework.
        Foundation::Framework *framework_;

        /// Pointer to RexLogicModule.
        /*boost::shared_ptr<RexLogicModule>*/ RexLogicModule* rexLogicModule_;

        /// Pointer to QtModule.
        boost::shared_ptr<QtUI::QtModule> qtModule_;

        /// Inventory (model) pointer.
        boost::shared_ptr<OpenSimProtocol::InventoryModel> inventory_;

        /// Inventory window widget.
        QWidget *inventoryWidget_;

        /// Canvas for the inventory window.
        boost::shared_ptr<QtUI::UICanvas> canvas_;

        ///
        OpenSimProtocol::InventoryModel *inventoryModel_;
    };
}

#endif
