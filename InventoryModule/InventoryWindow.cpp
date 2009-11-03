// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InventoryWindow.h"
#include "OpenSimInventoryDataModel.h"
#include "WebdavInventoryDataModel.h"
#include "PythonScriptModule.h"
#include "InventoryItemModel.h"
#include "QtModule.h"
#include "RexLogicModule.h"
#include "InventoryEvents.h"
#include "QtUtils.h"

#include <QtUiTools>
#include <QFile>
#include <QAbstractItemView>

namespace Inventory
{

InventoryWindow::InventoryWindow(Foundation::Framework *framework, RexLogic::RexLogicModule *rexLogic) :
    framework_(framework), rexLogicModule_(rexLogic), inventoryWidget_(0), inventoryItemModel_(0), treeView_(0),
    buttonClose_(0), buttonDownload_(0), buttonUpload_(0), buttonAddFolder_(0), buttonDeleteItem_(0),
    buttonRename_(0)
{
    // Get QtModule and create canvas
    qtModule_ = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
    if (!qtModule_.get())
        return;

    canvas_ = qtModule_->CreateCanvas(QtUI::UICanvas::External).lock();

    // Init Inventory Widget and connect close signal
    InitInventoryWindow();

    QObject::connect(buttonClose_, SIGNAL(clicked()), this, SLOT(Hide()));

    // Add local widget to canvas, setup initial size and title and show canvas
    canvas_->SetCanvasSize(305, 200);
    canvas_->SetCanvasWindowTitle(QString("Inventory"));
    canvas_->AddWidget(inventoryWidget_);

    // Add to control bar
    qtModule_->AddCanvasToControlBar(canvas_, QString("Inventory"));
}

// virtual
InventoryWindow::~InventoryWindow()
{
    SAFE_DELETE(inventoryItemModel_);
}

void InventoryWindow::Toggle()
{
    if (canvas_)
    {
        if (canvas_->IsHidden())
            canvas_->Show();
        else
            canvas_->Hide();
    }
}

void InventoryWindow::Hide()
{
    if (canvas_)
        canvas_->Hide();
}

void InventoryWindow::InitOpenSimInventoryTreeModel()
{
    if (inventoryItemModel_)
    {
        LogError("Inventory treeview has already item model set!");
        return;
    }

    OpenSimInventoryDataModel *dataModel = new OpenSimInventoryDataModel(rexLogicModule_);
    inventoryItemModel_ = new InventoryItemModel(dataModel);
    treeView_->setModel(inventoryItemModel_);

    QObject::connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));
}

void InventoryWindow::InitWebDavInventoryTreeModel(std::string identityUrl, std::string hostUrl)
{
    if (inventoryItemModel_)
    {
        LogError("Inventory treeview has already item model set!");
        return;
    }

    WebdavInventoryDataModel *dataModel = new WebdavInventoryDataModel(STD_TO_QSTR(identityUrl), STD_TO_QSTR(hostUrl));
    inventoryItemModel_ = new InventoryItemModel(dataModel);
    treeView_->setModel(inventoryItemModel_);

    QObject::connect(treeView_, SIGNAL( doubleClicked(const QModelIndex &) ), 
        inventoryItemModel_, SLOT( CurrentSelectionChanged(const QModelIndex &) ));

    QObject::connect(inventoryItemModel_, SIGNAL( AbstractInventoryItemSelected(AbstractInventoryItem *) ),
        dataModel, SLOT( ItemSelectedFetchContent(AbstractInventoryItem *) ));
}

void InventoryWindow::ResetInventoryTreeModel()
{
    SAFE_DELETE(inventoryItemModel_);
}

void InventoryWindow::UpdateActions()
{
//    bool hasSelection = !view->selectionModel()->selection().isEmpty();
    //removeRowAction->setEnabled(hasSelection);
    //removeColumnAction->setEnabled(hasSelection);

    bool hasCurrent = treeView_->selectionModel()->currentIndex().isValid();
    //insertRowAction->setEnabled(hasCurrent);

    if (hasCurrent)
        treeView_->closePersistentEditor(treeView_->selectionModel()->currentIndex());
}

void InventoryWindow::HandleInventoryDescendent(InventoryItemEventData *item_data)
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

    // Create new children (row) to the inventory view.
    if (!inventoryItemModel_->insertRows(index.row(), 1, index, item_data))
        return;

    UpdateActions();
}

void InventoryWindow::FetchInventoryDescendents(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    inventoryItemModel_->FetchInventoryDescendents(index);

    treeView_->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::AddFolder()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    FetchInventoryDescendents(index);

    // Next few lines not probably needed, but saved in case we will have multiple columns in the near future.
    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

    // Create new children (row) to the inventory view.
    if (!model->insertRow(0, index))
        return;

    treeView_->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::DeleteItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    // Delete row from the inventory view model.
    if (model->removeRow(index.row(), index.parent()))
        UpdateActions();
}

void InventoryWindow::RenameItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();

    if (treeView_->model()->flags(index) & Qt::ItemIsEditable)
        treeView_->edit(index);
}

void InventoryWindow::Upload()
{
    Core::StringList filenames = Foundation::QtUtils::GetOpenRexFileNames(Foundation::QtUtils::GetCurrentPath());
    if (filenames.empty())
        return;

    InventoryUploadEventData data;
    data.filenames = filenames;

    Foundation::EventManagerPtr eventManager = framework_->GetEventManager();
    Core::event_category_id_t event_category = eventManager->QueryEventCategory("Inventory");

    if (event_category != 0)
        eventManager->SendEvent(event_category, Inventory::Events::EVENT_INVENTORY_UPLOAD, &data);
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
    buttonDeleteItem_ = inventoryWidget_->findChild<QPushButton*>("pushButton_DeleteItem");
    buttonRename_ = inventoryWidget_->findChild<QPushButton*>("pushButton_Rename");
    treeView_ = inventoryWidget_->findChild<QTreeView*>("treeView");

    // Connect signals
    //QObject::connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));
    QObject::connect(treeView_, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(FetchInventoryDescendents(const QModelIndex &)));

    QObject::connect(buttonAddFolder_, SIGNAL(clicked(bool)), this, SLOT(AddFolder()));
    QObject::connect(buttonDeleteItem_, SIGNAL(clicked(bool)), this, SLOT(DeleteItem()));
    QObject::connect(buttonRename_, SIGNAL(clicked(bool)), this, SLOT(RenameItem()));
    QObject::connect(buttonUpload_, SIGNAL(clicked(bool)), this, SLOT(Upload()));
}

} // namespace Inventory
