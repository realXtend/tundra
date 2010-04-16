/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryWindow.cpp
 *  @brief  Inventory window. Should be totally unaware of the underlaying inventory data model.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InventoryWindow.h"
#include "InventoryModule.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"
#include "InventoryTreeView.h"
#include "InventoryAsset.h"

#include "Inworld/NotificationManager.h"
#include "Inworld/Notifications/MessageNotification.h"
#include "Inworld/Notifications/ProgressNotification.h"

#include <QUiLoader>
#include <QFile>
#include <QTreeView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QFileDialog>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
//#include <QMessageBox>

#include "MemoryLeakCheck.h"

namespace Inventory
{

InventoryWindow::InventoryWindow(InventoryModule *owner, QWidget *parent) :
    owner_(owner),
    QWidget(parent),
    mainWidget_(0),
    inventoryItemModel_(0),
    treeView_(0),
    lineEditSearch_(0),
    actionMenu_(0),
    actionDelete_(0),
    actionRename_(0),
    actionCut_(0),
    actionPaste_(0),
    actionNewFolder_(0),
    actionOpen_(0),
    actionProperties_(0),
    actionCopyAssetReference_(0),
    actionUpload_(0),
    actionDownload_(0),
    actionSeparator_(0),
//    offset_(0),
    lastUsedPath_(QDir::currentPath())
//    uploadCount_(0),
//    uploadWidget_(0)
{
    InitInventoryWindow();
    
}

// virtual
InventoryWindow::~InventoryWindow()
{
    SAFE_DELETE(inventoryItemModel_);
    SAFE_DELETE(treeView_);
    SAFE_DELETE(lineEditSearch_);
    SAFE_DELETE(mainWidget_);
    SAFE_DELETE(actionMenu_);
    SAFE_DELETE(actionDelete_);
    SAFE_DELETE(actionRename_);
    SAFE_DELETE(actionCut_);
    SAFE_DELETE(actionPaste_);
    SAFE_DELETE(actionNewFolder_);
    SAFE_DELETE(actionOpen_);
    SAFE_DELETE(actionCopyAssetReference_);
    SAFE_DELETE(actionUpload_);
    SAFE_DELETE(actionDownload_);
    SAFE_DELETE(actionSeparator_);

/*
    QMutableMapIterator<QString, QMessageBox *> it(downloadDialogs_);
    while(it.hasNext())
    {
        QMessageBox *msgBox = it.next().value();
        SAFE_DELETE(msgBox);
        it.remove();
    }
*/
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

    // Connect download progress signals.
    connect(inventory_model.get(), SIGNAL(DownloadStarted(const QString &, const QString &)),
        this, SLOT(OpenDownloadProgess(const QString &, const QString &)));

    connect(inventory_model.get(), SIGNAL(DownloadAborted(const QString &)),
        this, SLOT(AbortDownload(const QString &)));

    connect(inventory_model.get(), SIGNAL(DownloadCompleted(const QString &)),
        this, SLOT(FinishProgessNotification(const QString &)));

    connect(inventory_model.get(), SIGNAL(UploadStarted(const QString &)),
        this, SLOT(UploadStarted(const QString &)));

    connect(inventory_model.get(), SIGNAL(UploadFailed(const QString &, const QString &)),
        this, SLOT(UploadFailed(const QString &, const QString &)));

    connect(inventory_model.get(), SIGNAL(UploadCompleted(const QString &)),
        this, SLOT(FinishProgessNotification(const QString &)));

    // Connect selectionChanged
    connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));

    // Connect notification delayed sending to avoid ui thread problems when creating the notification widget
    connect(inventoryItemModel_->GetInventory(), SIGNAL(Notification(QString, int)), this, SLOT(CreateNotification(QString, int)), Qt::QueuedConnection);

//    connect(inventoryItemModel_, SIGNAL(dataChanged), treeView_, SLOT(

    ///\todo Hack: connect this signal only for regular OS, causes problems with WebDAV.
    if (owner_->GetInventoryDataModelType() == InventoryModule::IDMT_OpenSim)
        connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(ExpandFolder(const QModelIndex &)));
}

void InventoryWindow::ResetInventoryTreeModel()
{
    SAFE_DELETE(inventoryItemModel_);
}

void InventoryWindow::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange)
    {
        QString text = QApplication::translate("Inventory::InventoryWindow", "Inventory");
        this->setWindowTitle(text);
        QGraphicsProxyWidget* widget = this->graphicsProxyWidget();
        widget->setWindowTitle(text);
    }
    else
    {
        QWidget::changeEvent(e);
    }

}

void InventoryWindow::OpenItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();

    inventoryItemModel_->Open(index);

    treeView_->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    UpdateActions();
}

void InventoryWindow::ExpandFolder(const QModelIndex &index)
{
    inventoryItemModel_->Open(index);
    UpdateActions();
}

void InventoryWindow::OpenItemProperties()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    InventoryAsset *item = dynamic_cast<InventoryAsset *>(inventoryItemModel_->GetItem(index));
    if (!item)
        return;

    // InventoryModule manages item properties windows.
    emit OpenItemProperties(item->GetID());
}

void InventoryWindow::AddFolder()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    QAbstractItemModel *model = treeView_->model();

    inventoryItemModel_->Open(index);

    if (!inventoryItemModel_->InsertFolder(index.row(), index, "New Folder"))
        return;

    treeView_->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    UpdateActions();

    RenameItem();
}

void InventoryWindow::DeleteItem()
{
    ///\todo Delete for multiple items.
/*
    const QItemSelection &selection = treeView_->selectionModel()->selection();
    if (selection.isEmpty())
        return;
*/

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
    QStringList filenames = QFileDialog::getOpenFileNames(0, "Open", lastUsedPath_, RexTypes::rexFileFilters);
    if (filenames.empty())
        return;

    QString path = filenames.last();
    path.remove(path.lastIndexOf('/'), path.size() - path.lastIndexOf('/') + 1);
    QDir dir(path);
    if (dir.exists())
        lastUsedPath_ = path;

    QStringList itemNames;
    inventoryItemModel_->Upload(index, filenames, itemNames);
}

void InventoryWindow::UploadFiles(QStringList &filenames)
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (filenames.isEmpty())
        return;

    QStringList itemNames;
    inventoryItemModel_->Upload(index, filenames, itemNames);
}

void InventoryWindow::Download()
{
    const QItemSelection &selection = treeView_->selectionModel()->selection();
    if (selection.isEmpty())
        return;

    QString storePath = QFileDialog::getExistingDirectory(mainWidget_, "Select location for file download", lastUsedPath_);
    if (storePath.isEmpty())
        return;

    inventoryItemModel_->Download(storePath, selection);
}

void InventoryWindow::CopyAssetReference()
{
    inventoryItemModel_->CopyAssetReferenceToClipboard(treeView_->selectionModel()->currentIndex());
}

void InventoryWindow::Search(const QString &text)
{
    treeView_->expandAll();
    treeView_->keyboardSearch(text);
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (index.isValid())
        treeView_->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

void InventoryWindow::UpdateActions()
{
    ///\todo Cut, Copy, Paste: actionCut_, actionCopy_, actionCut_

    QModelIndex index = treeView_->selectionModel()->currentIndex();

    bool canAddFolder = !inventoryItemModel_->IsLibraryItem(index);
    actionNewFolder_->setEnabled(canAddFolder);

    bool editable = treeView_->model()->flags(index) & Qt::ItemIsEditable;
    actionDelete_->setEnabled(editable);
    actionRename_->setEnabled(editable);

    bool is_asset = inventoryItemModel_->GetItemType(index) == AbstractInventoryItem::Type_Asset;
    actionCopyAssetReference_->setEnabled(is_asset);
    actionOpen_->setEnabled(is_asset);
    actionProperties_->setEnabled(is_asset);

    bool hasCurrent = treeView_->selectionModel()->currentIndex().isValid();

    if (hasCurrent)
        treeView_->closePersistentEditor(treeView_->selectionModel()->currentIndex());
}

void InventoryWindow::OpenDownloadProgess(const QString &asset_id, const QString &name)
{
    // Make one custom widget as download manager, untill then disable this because its quite intrusive for a user to pop up dialogs
    // and they dont atm come to from and the position is calculated with inv window or something strange

/*
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, "", "Downloading asset " + asset_id, QMessageBox::Ok);
    msgBox->setModal(false);
    downloadDialogs_[asset_id] = msgBox;

    boost::shared_ptr<UiServices::UiModule> ui_module =
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (ui_module.get())
    {
        QPointF pos = mainWidget_->mapToGlobal(QPoint(0, 0));
        pos.setX(pos.x() + offset_);
        pos.setY(pos.y() + offset_);
        offset_ += 20;

        ui_module->GetSceneManager()->AddWidgetToScene(
            msgBox, UiServices::UiWidgetProperties(pos, msgBox->size(), Qt::Dialog, "Download: " + name, false));
    }

    msgBox->show();
*/

    // Ali find a way to update the download/upload process! just do controller->setValue(int)
    // Now its just sitting there at artificial 11%
    UiServices::ProgressController *progress_controller = new UiServices::ProgressController();
    emit Notification(new UiServices::ProgressNotification("Downloading " + name + " from inventory", progress_controller));
    progress_controller->Start(11);
    notification_progress_map_[asset_id] = progress_controller;
}

void InventoryWindow::AbortDownload(const QString &asset_id)
{
    ///\todo
}

void InventoryWindow::FinishProgessNotification(const QString &id)
{
/*
    QMessageBox *msgBox = downloadDialogs_.take(asset_id);
    if (msgBox)
        delete msgBox;
*/
    if (notification_progress_map_.contains(id))
    {
        notification_progress_map_[id]->Finish();
        notification_progress_map_.remove(id);
    }
}

void InventoryWindow::UploadStarted(const QString &filename)
{
    ///\todo Ali find a way to update the download/upload process! just do controller->setValue(int)
    /// Now its just sitting there at artificial 13%
    /// Pforce: Not possible to show progress for single asset with the current HTTP upload path -Ali.
    UiServices::ProgressController *progress_controller = new UiServices::ProgressController();
    emit Notification(new UiServices::ProgressNotification("Uploading " + filename + " to inventory", progress_controller));
    progress_controller->Start(13);
    notification_progress_map_[filename] = progress_controller;
}

void InventoryWindow::UploadFailed(const QString &filename, const QString &reason)
{
    if (notification_progress_map_.contains(filename))
    {
        notification_progress_map_[filename]->FailWithReason(reason);
        notification_progress_map_.remove(filename);
    }
}

void InventoryWindow::InitInventoryWindow()
{
    QUiLoader loader;
    loader.setLanguageChangeEnabled(true);
    QFile uiFile("./data/ui/inventory.ui");
    mainWidget_ = loader.load(&uiFile, this);
    uiFile.close();

    // Layout 
    layout_ = new QVBoxLayout(this);
    layout_->addWidget(mainWidget_);
    layout_->setContentsMargins(0, 0, 0, 0);
    setLayout(layout_);
    QLineEdit *lineEditSearch_ = new QLineEdit(mainWidget_);

    // Create inventory tree view.
    treeView_ = new InventoryTreeView(mainWidget_);
//    QHBoxLayout *hlayout = mainWidget_->findChild<QHBoxLayout *>("horizontalLayout_BottomContainer");
//    hlayout->addWidget(treeView_);
    layout_->addWidget(lineEditSearch_);
    layout_->addWidget(treeView_);

    // Connect signals
    ///\todo Connecting both these signals causes WebDav inventory to work incorrectly.
//    connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(ExpandFolder(const QModelIndex &)));
    connect(treeView_, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OpenItem()));

    QObject::connect(lineEditSearch_, SIGNAL(textChanged(const QString &)), this, SLOT(Search(const QString &)));

    CreateActions();
}

void InventoryWindow::CreateActions()
{
/*
    itemMenu_ = new QMenu(this);
    fileTransferMenu_ = new QMenu(this);
    connect(actionMenu_, SIGNAL(aboutToShow()), this, SLOT(UpdateActions()));
*/
    // File transfer actions
    actionUpload_= new InventoryAction(tr("&Upload"), this);
    actionUpload_->setStatusTip(tr("Upload file to your inventory"));
    connect(actionUpload_, SIGNAL(triggered()), this, SLOT(Upload()));
    treeView_->addAction(actionUpload_);

    actionDownload_= new InventoryAction(tr("&Download"), this);
    actionDownload_->setStatusTip(tr("Download assets to your hard drive"));
    connect(actionDownload_, SIGNAL(triggered()), this, SLOT(Download()));
    treeView_->addAction(actionDownload_);

    // Add separator
    actionSeparator_= new InventoryAction(this);
    actionSeparator_->setSeparator(true);
    treeView_->addAction(actionSeparator_);

    // Inventory item actions.
    actionDelete_ = new InventoryAction(tr("&Delete"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionDelete_->setStatusTip(tr("Delete this item"));
    connect(actionDelete_, SIGNAL(triggered()), this, SLOT(DeleteItem()));
    treeView_->addAction(actionDelete_);

    actionRename_ = new InventoryAction(tr("&Rename"), this);
    //actionRename_->setShortcuts();
    actionRename_->setStatusTip(tr("Rename this item"));
    connect(actionRename_, SIGNAL(triggered()), this, SLOT(RenameItem()));
    treeView_->addAction(actionRename_);

/*
    actionCut_ = new InventoryAction(tr("&Cut"), this);
    actionDelete_->setShortcuts(QKeySequence::Cut);
    actionDelete_->setStatusTip(tr("Cut this item"));
    connect(actionCut_, SIGNAL(triggered()), this, SLOT(Test()));
    treeView_->addAction(actionCut_);

    actionPaste_ = new InventoryAction(tr("&Paste"), this);
    actionDelete_->setShortcuts(QKeySequence::Paste);
    actionDelete_->setStatusTip(tr("Paste this item"));
    connect(actionPaste_, SIGNAL(triggered()), this, SLOT(Test()));
    treeView_->addAction(actionPaste_);
*/

    actionNewFolder_ = new InventoryAction(tr("&New folder"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionNewFolder_->setStatusTip(tr("Create new folder"));
    connect(actionNewFolder_, SIGNAL(triggered()), this, SLOT(AddFolder()));
    treeView_->addAction(actionNewFolder_);

    actionOpen_ = new InventoryAction(tr("&Open"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionOpen_->setStatusTip(tr("Open this item"));
    connect(actionOpen_, SIGNAL(triggered()), this, SLOT(OpenItem()));
    treeView_->addAction(actionOpen_);

    actionProperties_= new InventoryAction(tr("&Properties"), this);
    //actionProperties_->setShortcuts(QKeySequence::Delete);
    actionProperties_->setStatusTip(tr("View item properties"));
    connect(actionProperties_, SIGNAL(triggered()), this, SLOT(OpenItemProperties()));
    treeView_->addAction(actionProperties_);

    actionCopyAssetReference_ = new InventoryAction(tr("&Copy asset reference"), this);
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionCopyAssetReference_->setStatusTip(tr("Delete this item"));
    connect(actionCopyAssetReference_, SIGNAL(triggered()), this, SLOT(CopyAssetReference()));
    treeView_->addAction(actionCopyAssetReference_);
}

void InventoryWindow::CreateNotification(QString message, int hide_time)
{
    emit Notification(new UiServices::MessageNotification(message, hide_time));
}

} // namespace Inventory
