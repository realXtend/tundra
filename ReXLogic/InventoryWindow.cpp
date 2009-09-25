// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InventoryWindow.h"
#include "RexLogicModule.h"
#include "QtModule.h"
#include "UICanvas.h"
#include "InventoryModel.h"

#include <QFile>
#include <QLabel>
#include <QString>
#include <QtUiTools>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeView>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QDirModel>
#include <QAction>

namespace RexLogic
{

InventoryWindow::InventoryWindow(Foundation::Framework *framework, RexLogicModule *rexlogic) :
framework_(framework), rexLogicModule_(rexlogic), inventoryWidget_(0)
{
    InitInventoryWindow();
}

InventoryWindow::~InventoryWindow()
{
    inventoryWidget_ = 0;
}

void InventoryWindow::InitInventoryWindow()
{
    qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

    // If this occurs, we're most probably operating in headless mode.
    if (qtModule_.get() == 0)
        return;

    canvas_ = qtModule_->CreateCanvas(QtUI::UICanvas::External).lock();

    QUiLoader loader;
    QFile file("./data/ui/inventory.ui");
    if (!file.exists())
    {
        RexLogicModule::LogError ("Can't open login.UI file.");
        return;
    }

    inventoryWidget_ = loader.load(&file); 

    ///\todo Here we have strange and not-so-wanted feature.
    /// If you first addWidget (in Internal-canvas) and then SetCanvasSize() result: only partial window is seen.
    /// Must investigate futher that why this happends.

    // Set canvas size. 
    QSize size = inventoryWidget_->size();
    canvas_->SetCanvasSize(size.width(), size.height());
    canvas_->AddWidget(inventoryWidget_);

    // Create connections.
    QPushButton *button = inventoryWidget_->findChild<QPushButton *>("pushButtonRefresh");
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(InitInventoryTreeView()));

    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QObject::connect(treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
    QObject::connect(treeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
//    QObject::connect(treeView, SIGNAL(entered(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
//    QObject::connect(treeView, SIGNAL(pressed(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));


//    QObject::connect(treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
//        const QItemSelection &)), this, SLOT(updateActions()));

    QAction *closeAction = inventoryWidget_->findChild<QAction *>("closeAction");
    QObject::connect(closeAction, SIGNAL(triggered()), this, SLOT(Hide()));
//    QObject::connect(qApp, SIGNAL(triggered()), this, SLOT(Hide()));

//    QMenu *actionsMenu = inventoryWidget_->findChild<QMenu *>("actionsMenu");
//    QObject::connect(actionsMenu, SIGNAL(aboutToShow()), this, SLOT(updateActions()));

    QAction *createFolderAction = inventoryWidget_->findChild<QAction *>("createFolderAction");
    QObject::connect(createFolderAction, SIGNAL(triggered()), this, SLOT(CreateFolder()));

    QAction *deleteFolderAction = inventoryWidget_->findChild<QAction *>("deleteFolderAction");
    QObject::connect(deleteFolderAction, SIGNAL(triggered()), this, SLOT(DeleteFolder()));
}

void InventoryWindow::InitInventoryTreeView()
{
    inventory_  =  rexLogicModule_->GetInventory();
    if (!inventory_.get())
        return;

    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
//QDirModel *model = new QDirModel;
//    treeView->setModel(model);
    treeView->setModel(inventory_.get());
}

void InventoryWindow::Show()
{
    if (canvas_)
        canvas_->Show();
}

void InventoryWindow::Hide()
{
    if (canvas_)
        canvas_->Hide();
}

void InventoryWindow::FetchInventoryDescendents(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (!inventory_.get())
        return;

//    if (!dirty)
//        return;

    ///\todo Use id instead of the name.
    /*std::string name = index.data().toString().toStdString();
    OpenSimProtocol::InventoryFolder *folder  = inventory_->GetFirstChildFolderByName(name.c_str());
    rexLogicModule_->GetServerConnection()->SendFetchInventoryDescendents(folder->GetID(),
        folder->GetParent()->GetID(), 0 , true, true);
    */
}

void InventoryWindow::UpdateActions()
{
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    bool hasSelection = !treeView->selectionModel()->selection().isEmpty();

    QAction *createFolderAction = inventoryWidget_->findChild<QAction *>("createFolderAction");
    QAction *deleteFolderAction = inventoryWidget_->findChild<QAction *>("deleteFolderAction");
//    removeRowAction->setEnabled(hasSelection);
//    removeColumnAction->setEnabled(hasSelection);

    bool hasCurrent = treeView->selectionModel()->currentIndex().isValid();
//    insertRowAction->setEnabled(hasCurrent);
//    insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent)
    {
        treeView->closePersistentEditor(treeView->selectionModel()->currentIndex());
/*
        int row = view->selectionModel()->currentIndex().row();
        int column = view->selectionModel()->currentIndex().column();
        if (view->selectionModel()->currentIndex().parent().isValid())
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        else
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
*/
    }
}

void InventoryWindow::CreateFolder()
{
    using namespace OpenSimProtocol;

    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QModelIndex index = treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView->model();

    InventoryModel *inventoryModel = dynamic_cast<InventoryModel *>(model);
    if (!inventoryModel)
        return;

    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

    // Create new children (row) to the inventory view.
//    if (!model->insertRow(0, index))
//        return;

    InventoryFolder *newFolder = inventoryModel->InsertRow(0, index);
    if (!newFolder)
        return;

    // Inform the server.
    rexLogicModule_->GetServerConnection()->SendCreateInventoryFolderPacket(
        newFolder->GetParent()->GetID(), newFolder->GetID(), 255, newFolder->GetName());

/*    for(int column = 0; column < model->columnCount(index); ++column)
    {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("[New Folder]"), Qt::EditRole);

        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }
*/

    treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::DeleteFolder()
{
    return;

    using namespace OpenSimProtocol;

    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QModelIndex index = treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView->model();

    InventoryModel *inventoryModel = dynamic_cast<InventoryModel *>(model);
    if (!inventoryModel)
        return;

    // Inform the server.
    InventoryFolder *folder = static_cast<InventoryFolder *>(inventoryModel->GetItem(index));

    // When deleting folders, we move them first to the Trash folder.
    InventoryFolder *trashFolder = inventory_->GetTrashFolder();
    if (!trashFolder)
        return; // Should not happen.

    // If the folder is already in the trash folder, delete it for good.
    if (folder->GetParent() == trashFolder)
        rexLogicModule_->GetServerConnection()->SendRemoveInventoryFolderPacket(folder->GetID());
    else
    {
        rexLogicModule_->GetServerConnection()->SendMoveInventoryFolderPacket(folder->GetID(), trashFolder->GetID());
        std::cout << "Moving folder " << folder->GetID() << "to" << trashFolder->GetID() << std::endl;
    }

    // Delete row from the inventory view model.
    if (model->removeRow(index.row(), index.parent()))
        UpdateActions();

    // Delete the real folder.
    inventory_->GetRoot()->DeleteChild(folder);
}

} // namespace RexLogic
