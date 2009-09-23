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
        InventoryWindow(Foundation::Framework *framework, RexLogicModule *module);

        /// Destructor.
        virtual ~InventoryWindow();

    public slots:
        void PopulateInventoryView();
        void OpenFolder(QTreeWidgetItem* item, int column);
        void CloseFolder();
        void OpenItem();
        void FetchInventoryDescendents(const QModelIndex &index);
        void UpdateActions();

    private slots:
//        void CreateFolder();
//        bool InsertColumn(const QModelIndex &parent = QModelIndex());
        void InsertRow();
//        bool RemoveColumn(const QModelIndex &parent = QModelIndex());
        void RemoveRow();

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
