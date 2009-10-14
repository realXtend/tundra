// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InventoryWindow.h"
#include "OpenSimInventoryDataModel.h"
#include "InventoryViewModel.h"
#include "QtModule.h"
#include "RexLogicModule.h"
#include "NetworkEvents.h"

#include <QtUiTools>
#include <QFile>
#include <QDebug>

namespace Inventory
{

InventoryWindow::InventoryWindow(Foundation::Framework *framework, RexLogic::RexLogicModule *rexLogic) :
    framework_(framework), rexLogicModule_(rexLogic), inventoryWidget_(0), viewModel_(0), treeView_(0),
    buttonClose_(0), buttonDownload_(0), buttonUpload_(0), buttonAddFolder_(0), buttonDeleteFolder_(0),
    buttonRename_(0)
{
    // Get QtModule and create canvas
    qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
    if (!qtModule_.get())
        return;

    canvas_ = qtModule_->CreateCanvas(QtUI::UICanvas::External).lock();

    // Init Inventory Widget and connect close signal
    InitInventoryWindow();

    QObject::connect(buttonClose_, SIGNAL(clicked()), this, SLOT(CloseInventoryWindow()));

    // Add local widget to canvas, setup initial size and title and show canvas
    canvas_->SetCanvasSize(300, 275);
    canvas_->SetCanvasWindowTitle(QString("Inventory"));
    canvas_->AddWidget(inventoryWidget_);
}

// virtual
InventoryWindow::~InventoryWindow()
{
    SAFE_DELETE(viewModel_);
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

void InventoryWindow::InitOpenSimInventoryTreeModel()
{
    if (viewModel_)
    {
        LogError("Inventory treeview has already view model set!");
        return;
    }

    OpenSimInventoryDataModel *dataModel = new OpenSimInventoryDataModel(rexLogicModule_);
    viewModel_ = new InventoryViewModel(dataModel);
    treeView_->setModel(viewModel_);

    // Connect view model related signals.
    QObject::connect(viewModel_, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
        this, SLOT(ItemNameChanged(const QModelIndex &, const QModelIndex &)));

    QObject::connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));
}

void InventoryWindow::ResetInventoryTreeModel()
{
    SAFE_DELETE(viewModel_);
//    CloseInventoryWindow();
}

void InventoryWindow::UpdateActions()
{
    ///\todo
}

void InventoryWindow::HandleInventoryFolderDescendents(OpenSimProtocol::InventoryFolderEventData *folder_data)
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();

    QAbstractItemModel *model = treeView_->model();

    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

//    viewModel_->SetPendingData(folder_data);

    // Create new children (row) to the inventory view.
    //inline bool QAbstractItemModel::insertRow(int arow, const QModelIndex &aparent) { return insertRows(arow, 1, aparent); }
    if (!viewModel_->insertRows(index.row(), 1, index, folder_data))
    //if (!viewModel_->insertRow(0, index))
        return;

    UpdateActions();
}

void InventoryWindow::FetchInventoryDescendents(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    viewModel_->FetchInventoryDescendents(index);

    treeView_->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::AddFolder()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

    // Create new children (row) to the inventory view.
    if (!model->insertRow(0, index))
        return;

    treeView_->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::DeleteFolder()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    // Delete row from the inventory view model.
    if (model->removeRow(index.row(), index.parent()))
        UpdateActions();
}

void InventoryWindow::ItemNameChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
/*
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
            folder->GetID(), folder->GetParent()->GetID(), 127/*folder->GetType()*///, new_name);
/*    }
    else if (item->GetInventoryItemType() == Type_Asset)
    {
        ///\todo Handle asset name change.
//        InventoryAsset *asset = static_cast<InventoryAsset *>(item);
//        rexLogicModule_->GetServerConnection()->SendUpdateInventoryItemPacket(
//            folder->GetID(), folder->GetParent()->GetID(), folder->GetType(), new_name);
    }
    */
}

void InventoryWindow::CloseInventoryWindow()
{
    if (qtModule_.get() != 0)
        qtModule_.get()->DeleteCanvas(canvas_->GetID());
    
}

void InventoryWindow::InitInventoryWindow()
{
    // Create widget from ui file
    QUiLoader loader;
    QFile uiFile("./data/ui/inventory_main.ui");
    inventoryWidget_ = loader.load(&uiFile, 0);
    uiFile.close();

    // Get controls
    buttonClose_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Close");
    buttonDownload_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Download");
    buttonUpload_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Upload");
    buttonAddFolder_ = inventoryWidget_->findChild<QPushButton*>("pushButton_AddFolder");
    buttonDeleteFolder_ = inventoryWidget_->findChild<QPushButton*>("pushButton_DeleteFolder");
    buttonRename_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Rename");
    treeView_ = inventoryWidget_->findChild<QTreeView*>("treeView");

    // Connect signals
    QObject::connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
    QObject::connect(treeView_, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));

    QObject::connect(buttonAddFolder_, SIGNAL(clicked(bool)), this, SLOT(AddFolder()));
    QObject::connect(buttonDeleteFolder_, SIGNAL(clicked(bool)), this, SLOT(DeleteFolder()));
    QObject::connect(buttonRename_, SIGNAL(clicked(bool)), this, SLOT(RenameItem()));
}

} // namespace Inventory
