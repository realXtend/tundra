// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InventoryWindow.h"
#include "InventoryModule.h"
#include "AssetUploader.h"
#include "OpenSimInventoryDataModel.h"
#include "WebdavInventoryDataModel.h"
#include "InventoryItemModel.h"
#include "QtModule.h"
#include "RexLogicModule.h"
#include "Inventory/InventoryEvents.h"
#include "QtUtils.h"
#include "AssetEvents.h"
#include "TextureInterface.h"

#include <QtUiTools>
#include <QFile>
#include <QAbstractItemView>
#include <QModelIndex>

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

    canvas_ = qtModule_->CreateCanvas(QtUI::UICanvas::Internal).lock();

    // Init Inventory widget and connect close signal
    InitInventoryWindow();

    // Add local widget to canvas, setup initial size and title and show canvas
    canvas_->SetSize(300, 350);
    canvas_->SetStationary(false);
    //canvas_->SetResizable(false);
    canvas_->SetPosition(canvas_->GetRenderWindowSize().width() - 350, 35);
    canvas_->SetWindowTitle(QString("Inventory"));
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

void InventoryWindow::SetWorldStreamToDataModel(ProtocolUtilities::WorldStreamPtr world_stream)
{
    if (inventoryItemModel_)
        inventoryItemModel_->GetInventory()->SetWorldStream(world_stream);
}

void InventoryWindow::InitOpenSimInventoryTreeModel(InventoryModule *inventory_module,ProtocolUtilities::WorldStreamPtr world_stream)
{
    if (inventoryItemModel_)
    {
        LogError("Inventory treeview has already item model set!");
        return;
    }

    OpenSimInventoryDataModel *dataModel = new OpenSimInventoryDataModel(rexLogicModule_);
    dataModel->SetWorldStream(world_stream);
    inventoryItemModel_ = new InventoryItemModel(dataModel);
    inventoryItemModel_->SetUseTrash(true);
    treeView_->setModel(inventoryItemModel_);

    QObject::connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));

    QObject::connect(this, SIGNAL(FileUpload(Core::StringList&)), inventory_module->GetAssetUploader().get(),
        SLOT(UploadFiles(Core::StringList&)));

    inventoryType_ = "OpenSim";
}

void InventoryWindow::InitWebDavInventoryTreeModel(const std::string &identityUrl, const std::string &hostUrl)
{
    if (inventoryItemModel_)
    {
        LogError("Inventory treeview has already item model set!");
        return;
    }

    // Create webdav inventory model
    WebdavInventoryDataModel *dataModel = new WebdavInventoryDataModel(STD_TO_QSTR(identityUrl), STD_TO_QSTR(hostUrl));
    inventoryItemModel_ = new InventoryItemModel(dataModel);
    treeView_->setModel(inventoryItemModel_);

    // Connect signals
    QObject::connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));

//    QObject::connect(treeView_, SIGNAL(doubleClicked(const QModelIndex &)),
//        inventoryItemModel_, SLOT(CurrentSelectionChanged(const QModelIndex &)));

//    QObject::connect(inventoryItemModel_, SIGNAL(AbstractInventoryItemSelected(AbstractInventoryItem *)),
//        dataModel, SLOT(ItemSelectedFetchContent(AbstractInventoryItem *)));

    // Set inventory type
    inventoryType_ = "Webdav";
}

void InventoryWindow::ResetInventoryTreeModel()
{
    SAFE_DELETE(inventoryItemModel_);
}

void InventoryWindow::HandleInventoryDescendent(InventoryItemEventData *item_data)
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

    // Create new children (row) to the inventory view.
    if (!inventoryItemModel_->InsertItem(index.row(), index, item_data))
        return;

    UpdateActions();
}

void InventoryWindow::HandleResourceReady(const bool &resource, Foundation::EventDataInterface *data)
{
    if (inventoryType_ != "OpenSim")
        return;

    QString asset_id;
    if (resource)
    {
        Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady *>(data);
        asset_id = event_data->id_.c_str();
    }
    else
    {
        Asset::Events::AssetReady *event_data = checked_static_cast<Asset::Events::AssetReady*>(data);
        asset_id = event_data->asset_->GetId().c_str();
    }

    OpenSimInventoryDataModel *dataModel = dynamic_cast<OpenSimInventoryDataModel *>(inventoryItemModel_->GetInventory());
    if (!dataModel)
        return;

    if (dataModel->HasPendingDownloadRequests())
        dataModel->SaveAssetToDisk(data);
}

void InventoryWindow::ExpandFolder(const QModelIndex &index)
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

    inventoryItemModel_->FetchInventoryDescendents(index);

    // Next few lines not probably needed, but saved in case we will have multiple columns in the near future.
    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

    bool ok = false;
    QString newFolderName = QInputDialog::getText(canvas_->GetView(), "Create New Folder",
        "Please give name of the new folder", QLineEdit::Normal, "", &ok);
    if (!ok)
        return;

    if (newFolderName.isEmpty())
        newFolderName = "New Folder";

    if (!inventoryItemModel_->InsertFolder(index.row(), index, newFolderName))
        return;

    treeView_->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::DeleteItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

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
    ///\todo make uploads for both models trough the same interface.
    if (inventoryType_ == "OpenSim")
    {
        Core::StringList filenames = Foundation::QtUtils::GetOpenRexFileNames(Foundation::QtUtils::GetCurrentPath());
        if (filenames.empty())
            return;

        emit FileUpload(filenames);
    }
    if(inventoryType_ == "Webdav")
    {
        QString filename(Foundation::QtUtils::GetOpenFileName("All Files (*.*)", "Select file for upload",
            Foundation::QtUtils::GetCurrentPath()).c_str());

        QModelIndex index = treeView_->selectionModel()->currentIndex();
        QStringList filenames;
        filenames << filename;

        inventoryItemModel_->Upload(index, filenames);
    }
}

void InventoryWindow::Download()
{
    const QItemSelection &selection = treeView_->selectionModel()->selection();
    if (selection.isEmpty())
        return;

    QString storePath = QFileDialog::getExistingDirectory(inventoryWidget_, "Select location for file download",
        QString(Foundation::QtUtils::GetCurrentPath().c_str()));
    if (storePath.isEmpty())
        return;

    inventoryItemModel_->Download(storePath, selection);
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

void InventoryWindow::InitInventoryWindow()
{
    // Create widget from ui file
    QUiLoader loader;
    QFile uiFile("./data/ui/inventory_main.ui");
    inventoryWidget_ = loader.load(&uiFile, 0);
    inventoryWidget_->resize(300, 350);
    uiFile.close();

    // Get controls
    buttonClose_ = inventoryWidget_->findChild<QPushButton *>("pushButton_Close");
    buttonDownload_ = inventoryWidget_->findChild<QPushButton *>("pushButton_Download");
    buttonUpload_ = inventoryWidget_->findChild<QPushButton *>("pushButton_Upload");
    buttonAddFolder_ = inventoryWidget_->findChild<QPushButton *>("pushButton_AddFolder");
    buttonDeleteItem_ = inventoryWidget_->findChild<QPushButton *>("pushButton_DeleteItem");
    buttonRename_ = inventoryWidget_->findChild<QPushButton *>("pushButton_Rename");
    treeView_ = inventoryWidget_->findChild<QTreeView *>("treeView");

    // Connect signals
    ///\todo Connecting both these signals causes WebDav inventory to work incorrectly.
//    QObject::connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(ExpandFolder(const QModelIndex &)));
    QObject::connect(treeView_, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(ExpandFolder(const QModelIndex &)));

    QObject::connect(buttonClose_, SIGNAL(clicked()), this, SLOT(Hide()));
    QObject::connect(buttonAddFolder_, SIGNAL(clicked(bool)), this, SLOT(AddFolder()));
    QObject::connect(buttonDeleteItem_, SIGNAL(clicked(bool)), this, SLOT(DeleteItem()));
    QObject::connect(buttonRename_, SIGNAL(clicked(bool)), this, SLOT(RenameItem()));
    QObject::connect(buttonUpload_, SIGNAL(clicked(bool)), this, SLOT(Upload()));
    QObject::connect(buttonDownload_, SIGNAL(clicked(bool)), this, SLOT(Download()));
}

} // namespace Inventory
