// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryWindow.cpp
 *  @brief The inventory window.
 */

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
#include <QMenu>
#include <QStatusBar>

namespace RexLogic
{

using namespace OpenSimProtocol;

InventoryWindow::InventoryWindow(Foundation::Framework *framework, RexLogicModule *rexlogic) :
    framework_(framework), rexLogicModule_(rexlogic), inventoryWidget_(0), inventoryModel_(0)
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
    // Widgets
//    QPushButton *button = inventoryWidget_->findChild<QPushButton *>("pushButtonUpload");
//    QObject::connect(button, SIGNAL(clicked()), this, SLOT(Upload()));

    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QObject::connect(treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
    QObject::connect(treeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
//    QObject::connect(treeView, SIGNAL(entered(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
//    QObject::connect(treeView, SIGNAL(pressed(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));

    QMenu *actionsMenu = inventoryWidget_->findChild<QMenu *>("actionsMenu");
    QObject::connect(actionsMenu, SIGNAL(aboutToShow()), this, SLOT(UpdateActions()));

    // Actions
    QAction *actionClose = inventoryWidget_->findChild<QAction *>("actionClose");
    QObject::connect(actionClose, SIGNAL(triggered()), this, SLOT(Hide()));
//    QObject::connect(qApp, SIGNAL(triggered()), this, SLOT(Hide()));

    QAction *actionCreateFolder = inventoryWidget_->findChild<QAction *>("actionCreateFolder");
    QObject::connect(actionCreateFolder, SIGNAL(triggered()), this, SLOT(CreateFolder()));

    QAction *actionDeleteFolder = inventoryWidget_->findChild<QAction *>("actionDeleteFolder");
    QObject::connect(actionDeleteFolder, SIGNAL(triggered()), this, SLOT(DeleteFolder()));

//    QAction *actionRename = inventoryWidget_->findChild<QAction *>("actionRename");
//    QObject::connect(actionRename, SIGNAL(triggered()), this, SLOT(Rename()));
}

void InventoryWindow::InitInventoryTreeView()
{
    inventory_  =  rexLogicModule_->GetInventory();
    if (!inventory_.get())
        return;

    ///\todo This is hackish. Refactor InventoryModel class into two different classes.
    inventoryModel_ = new InventoryModel(*inventory_.get());
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    treeView->setModel(inventoryModel_);

    ///\todo Not maybe the best place to connect the signal?
    QObject::connect(inventoryModel_, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
        this, SLOT(NameChanged(const QModelIndex &, const QModelIndex &)));

//    QObject::connect(treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
//        const QItemSelection &)), this, SLOT(UpdateActions()));
}

void InventoryWindow::ResetInventoryTreeView()
{
    ///\todo Is this ok?
    inventoryModel_ = 0;
    //SAFE_DELETE(inventoryModel_);
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

void InventoryWindow::UpdateActions()
{
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QModelIndex index = treeView->selectionModel()->currentIndex();
    bool hasSelection = !treeView->selectionModel()->selection().isEmpty();

//    removeRowAction->setEnabled(hasSelection);
//    removeColumnAction->setEnabled(hasSelection);

    QAction *actionCreateFolder = inventoryWidget_->findChild<QAction *>("actionCreateFolder");
    QAction *actionDeleteFolder = inventoryWidget_->findChild<QAction *>("actionDeleteFolder");

    // Is this item editable?
    bool editable = inventory_->IsEditable(index);
    actionDeleteFolder->setEnabled(editable);
//    actionRename->setEnabled(editable);

    InventoryItemBase *item = inventory_->GetItem(index);
    if (item->IsEditable())
        actionDeleteFolder->setEnabled(true);

    if (item->GetInventoryItemType() == Type_Asset)
    {
        actionDeleteFolder->setEnabled(false);
        actionCreateFolder->setEnabled(false);
    }

    bool hasCurrent = treeView->selectionModel()->currentIndex().isValid();
    actionCreateFolder->setEnabled(hasCurrent);
//    insertColumnAction->setEnabled(hasCurrent);

    if (hasCurrent)
    {
        treeView->closePersistentEditor(treeView->selectionModel()->currentIndex());
        /*QStatusBar *statusBar = inventoryWidget_->findChild<QStatusBar *>("statusBar");
        if (item->GetInventoryItemType() == Type_Folder)
            statusBar->showMessage(tr("%1").arg("Folder"));
        else if(item->GetInventoryItemType() == Type_Asset)
            statusBar->showMessage(tr("%1").arg("Asset"));
        */
    }
}

void InventoryWindow::FetchInventoryDescendents(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (!inventory_.get())
        return;

    ///\todo Use id instead of the name.
    std::string name = index.data().toString().toStdString();
    OpenSimProtocol::InventoryFolder *folder  = inventory_->GetFirstChildFolderByName(name.c_str());

    ///\todo Send FetchInventoryDescendents only if our model is "dirty" (new items are uploaded)
//    if (!folder->IsDirty())
//        return;

    rexLogicModule_->GetServerConnection()->SendFetchInventoryDescendentsPacket(folder->GetID(),
        folder->GetParent()->GetID(), 0 , true, false);

    rexLogicModule_->GetServerConnection()->SendFetchInventoryDescendentsPacket(folder->GetID(),
        folder->GetParent()->GetID(), 0 , false, true);

//    treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::CreateFolder()
{
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

    treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::DeleteFolder()
{
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QModelIndex index = treeView->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView->model();

    if (!inventory_)
        return;

    InventoryFolder *folder = static_cast<InventoryFolder *>(inventory_->GetItem(index));

    // When deleting folders, we move them first to the Trash folder.
    // If the folder is already in the trash folder, delete it for good.
    ///\todo Move the "deleted" folder to the Trash folder and update the view.
    InventoryFolder *trashFolder = inventory_->GetTrashFolder();
    if (!trashFolder)
    {
        RexLogicModule::LogError("Can't find Trash folder. Moving folder to Trash not possible.");
        return;
    }

    if (folder->GetParent() == trashFolder)
        rexLogicModule_->GetServerConnection()->SendRemoveInventoryFolderPacket(folder->GetID());
    else
        rexLogicModule_->GetServerConnection()->SendMoveInventoryFolderPacket(folder->GetID(), trashFolder->GetID());

    // Delete row from the inventory view model.
    if (model->removeRow(index.row(), index.parent()))
        UpdateActions();
}

void InventoryWindow::CreateAsset()
{
    // Add some handler somewhere which fires an event when  a succesfull asset upload has occured.
    // Use the inventory and asset id's and create new InventoryAsset object.
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

    treeView->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::DeleteAsset()
{
/*
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QModelIndex index = treeView->selectionModel()->currentIndex();
    InventoryAsset *asset = static_cast<InventoryAsset *>(inventoryModel->GetItem(index));
    rexLogicModule_->GetServerConnection()->SendRemoveInventoryItemPacket(asset->GetID());
*/
}

void InventoryWindow::NameChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QTreeView *treeView = inventoryWidget_->findChild<QTreeView *>("treeView");
    QAbstractItemModel *model = treeView->model();

    if (topLeft != bottomRight)
        return;

    std::string new_name = topLeft.data().toString().toStdString();

    InventoryItemBase *item = inventory_->GetItem(topLeft);
    std::cout << "item anem " << item->GetName() << std::endl;

    if (item->GetInventoryItemType() == Type_Folder)
    {
        InventoryFolder *folder = static_cast<InventoryFolder *>(item);
        rexLogicModule_->GetServerConnection()->SendUpdateInventoryFolderPacket(
            folder->GetID(), folder->GetParent()->GetID(), 127/*folder->GetType()*/, new_name);
    }
    else if (item->GetInventoryItemType() == Type_Asset)
    {
        ///\todo Handle asset name change.
//        InventoryAsset *asset = static_cast<InventoryAsset *>(item);
//        rexLogicModule_->GetServerConnection()->SendUpdateInventoryItemPacket(
//            folder->GetID(), folder->GetParent()->GetID(), folder->GetType(), new_name);
    }
}

} // namespace RexLogic
