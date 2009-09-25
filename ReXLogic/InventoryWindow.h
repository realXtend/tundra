// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InventoryWindow_h
#define incl_InventoryWindow_h

//#include "NetworkEvents.h"
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
        ///
        void InitInventoryTreeView();

        /// Shows the inventory window.
        void Show();

        /// Hides the inventory window.
        void Hide();

        ///
        void FetchInventoryDescendents(const QModelIndex &index);

        /// Updates menu actions.
        void UpdateActions();

    private slots:
        /// Creates new folder.
        void CreateFolder();

        /// Deletes folder.
        void DeleteFolder();

    private:
        ///
        void InitInventoryWindow();

        /// Pointer to the framework.
        Foundation::Framework *framework_;

        /// Pointer to RexLogicModule.
        /*boost::shared_ptr<RexLogicModule>*/ RexLogicModule* rexLogicModule_;

        /// Pointer to QtModule.
        boost::shared_ptr<QtUI::QtModule> qtModule_;

        ///
        boost::shared_ptr<OpenSimProtocol::InventoryModel> inventory_;

        ///
        QWidget *inventoryWidget_;

        ///
        boost::shared_ptr<QtUI::UICanvas> canvas_;

        ///
        boost::shared_ptr<QtUI::UICanvas> screen_canvas_;
    };
} // namespace RexLogic

#endif
