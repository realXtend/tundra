// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryWindow.cpp
 *  @brief The inventory window.
 */

#include "StableHeaders.h"
#include "InventoryWindow.h"
#include "InventoryModule.h"
#include "Framework.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"
#include "QtModule.h"
#include "RexLogicModule.h"
#include "Inventory/InventoryEvents.h"
#include "QtUtils.h"

#include <QUiLoader>
#include <QFile>
#include <QTreeView>
#include <QPushButton>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QInputDialog>
#include <QFileDialog>

namespace Inventory
{

InventoryWindow::InventoryWindow(Foundation::Framework *framework) :
    framework_(framework),
    inventoryWidget_(0),
    inventoryItemModel_(0),
    treeView_(0),
    buttonClose_(0),
    buttonDownload_(0),
    buttonUpload_(0),
    buttonAddFolder_(0),
    buttonDeleteItem_(0),
    buttonRename_(0)
{
    // Get QtModule and create canvas
     boost::shared_ptr<QtUI::QtModule> qtModule =
        framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
    if (!qtModule.get())
        return;

    canvas_ = qtModule->CreateCanvas(QtUI::UICanvas::Internal).lock();

    InitInventoryWindow();

    // Add local widget to canvas, setup initial size and title.
    canvas_->SetSize(300, 350);
    canvas_->SetStationary(false);
    canvas_->SetPosition(canvas_->GetRenderWindowSize().width() - 350, 35);
    canvas_->SetWindowTitle(QString("Inventory"));
    canvas_->AddWidget(inventoryWidget_);

    // Add to control bar
    qtModule->AddCanvasToControlBar(canvas_, QString("Inventory"));
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

void InventoryWindow::InitInventoryTreeModel(InventoryPtr inventory_model)
{
    if (inventoryItemModel_)
    {
        InventoryModule::LogError("Inventory treeview has already item model set!");
        return;
    }

    // Create inventory item model.
    inventoryItemModel_ = new InventoryItemModel(inventory_model.get());
    treeView_->setModel(inventoryItemModel_);

    QObject::connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));
}

void InventoryWindow::ResetInventoryTreeModel()
{
    SAFE_DELETE(inventoryItemModel_);
}

/*
void InventoryWindow::HandleInventoryDescendent(InventoryItemEventData *item_data)
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    ///\todo Can be removed?
    if (model->columnCount(index) == 0)
        if (!model->insertColumn(0, index))
            return;

    if (!inventoryItemModel_->InsertItem(index.row(), index, item_data))
        return;

    UpdateActions();
}
*/

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

    ///\todo    Change the functionality so that new user Add Folder it creates new folder but doesn't notify
    ///         server until the name has been given.
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
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    Core::StringList names = Foundation::QtUtils::GetOpenRexFileNames(Foundation::QtUtils::GetCurrentPath());
    if (names.empty())
        return;

    // Convert to QStringList ///\todo Use QStringList all the way...
    QStringList filenames;
    for (Core::StringList::iterator it = names.begin(); it != names.end(); ++it)
        filenames << QString((*it).c_str());

    inventoryItemModel_->Upload(index, filenames);
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
    ///\todo Utilize this function properly.
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
