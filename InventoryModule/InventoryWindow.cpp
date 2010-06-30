/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryWindow.cpp
 *  @brief  Inventory window. Should be totally unaware of the underlaying inventory data model.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InventoryWindow.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"
#include "InventoryTreeView.h"
#include "InventoryAsset.h"

#include "Inworld/NotificationManager.h"
#include "Inworld/Notifications/MessageNotification.h"
#include "Inworld/Notifications/ProgressNotification.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("InventoryWindow")

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
#include <QLineEdit>
//#include <QMessageBox>

#include "MemoryLeakCheck.h"

namespace Inventory
{

InventoryWindow::InventoryWindow(QWidget *parent) :
    QWidget(parent),
    mainWidget_(0),
    inventoryItemModel_(0),
    treeView_(0),
    lineEditSearch_(0),
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
        LogError("Inventory treeview has already item model set!");
        return;
    }

    // Create inventory item model.
    inventoryItemModel_ = new InventoryItemModel(inventory_model.get());
    treeView_->setModel(inventoryItemModel_);

    // Connect view/selection-related signals.
    connect(inventoryItemModel_, SIGNAL(IndexModelIsDirty(const QModelIndex &)),
        this, SLOT(IndexIsDirty(const QModelIndex &)));

    connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(ExpandFolder(const QModelIndex &)));

    connect(treeView_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,
        const QItemSelection &)), this, SLOT(UpdateActions()));

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

void InventoryWindow::IndexIsDirty(const QModelIndex &index)
{
    treeView_->collapse(index);
    inventoryItemModel_->Open(index);
    treeView_->expand(index);

    /**\todo
        This is hack to get around Qt bug introduced in 4.6.0:
        When dynamically adding new children for folder the expand arrow will not become visible
        until we've collapsed and expanded the parent folder also.
        When we've updated to newer version, try if this is needed anymore.
    QModelIndex parentIndex = inventoryItemModel_->parent(index);
    treeView_->collapse(parentIndex);
    treeView_->expand(parentIndex);
    */
}

void InventoryWindow::OpenItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (AbstractInventoryItem::Type_Folder == inventoryItemModel_->GetItemType(index))
    {
        if (!treeView_->isExpanded(index))
            inventoryItemModel_->Open(index);
    }
    else
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

    QString id = inventoryItemModel_->GetItemId(index);
    if (id.isEmpty())
        return;

    // InventoryModule manages item properties windows.
    emit OpenItemProperties(id);
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

    inventoryItemModel_->CheckTreeForDirtys();
}

void InventoryWindow::DeleteItem()
{
/// @todo Delete for multiple items. Currenly causes crash in InventoryItemModel::parent()
/*
    const QItemSelection &selection = treeView_->selectionModel()->selection();
    if (selection.isEmpty())
        return;

    QListIterator<QModelIndex> it(selection.indexes());
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        if (treeView_->model()->removeRow(index.row(), index.parent()))
            UpdateActions();
    }
*/
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (!index.isValid())
        return;

    if (treeView_->model()->removeRow(index.row(), index.parent()))
        UpdateActions();

    inventoryItemModel_->CheckTreeForDirtys();
}

void InventoryWindow::RenameItem()
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (treeView_->model()->flags(index) & Qt::ItemIsEditable)
        treeView_->edit(index);

    inventoryItemModel_->CheckTreeForDirtys();
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

    inventoryItemModel_->CheckTreeForDirtys();
}

void InventoryWindow::UploadFiles(QStringList &filenames)
{
    QModelIndex index = treeView_->selectionModel()->currentIndex();
    if (filenames.isEmpty())
        return;

    QStringList itemNames;
    inventoryItemModel_->Upload(index, filenames, itemNames);

    inventoryItemModel_->CheckTreeForDirtys();
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
    ///\todo Cut, Copy, Paste: actionCut, actionCopy, actionCut
    InventoryAction *actionNewFolder = 0, *actionDelete = 0, *actionRename = 0,
        *actionCopyAssetReference = 0, *actionOpen = 0, *actionProperties = 0;

    actionNewFolder = findChild<InventoryAction *>("NewFolder");
    actionDelete = findChild<InventoryAction *>("Delete");
    actionRename = findChild<InventoryAction *>("Rename");
    actionCopyAssetReference = findChild<InventoryAction *>("CopyAssetReference");
    actionOpen = findChild<InventoryAction *>("Open");
    actionProperties = findChild<InventoryAction *>("Properties");

    QModelIndex index = treeView_->selectionModel()->currentIndex();

    bool canAddFolder = !inventoryItemModel_->IsLibraryItem(index);
    if (actionNewFolder)
        actionNewFolder->setEnabled(canAddFolder);

    bool editable = treeView_->model()->flags(index) & Qt::ItemIsEditable;
    if (actionDelete)
        actionDelete->setEnabled(editable);
    if (actionRename)
        actionRename->setEnabled(editable);

    bool is_asset = inventoryItemModel_->GetItemType(index) == AbstractInventoryItem::Type_Asset;
    if (actionCopyAssetReference)
        actionCopyAssetReference->setEnabled(is_asset);
    if (actionOpen)
        actionOpen->setEnabled(is_asset);
    if (actionProperties)
        actionProperties->setEnabled(is_asset);

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
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
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

    ///\todo Find a way to update the download process if possible
    UiServices::ProgressController *progress_controller = new UiServices::ProgressController();
    emit Notification(new UiServices::ProgressNotification("Downloading " + name + " from inventory", progress_controller));
    progress_controller->Start(11);
    notification_progress_map_[asset_id] = progress_controller;
}

void InventoryWindow::AbortDownload(const QString &asset_id)
{
    ///\note Aborting not possible with the current protocol.
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
    /// No way to have any real upload progress info with current current HTTP upload path.
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

void InventoryWindow::CreateNotification(QString message, int hide_time)
{
    emit Notification(new UiServices::MessageNotification(message, hide_time));
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
    InventoryAction  *actionUpload= new InventoryAction(tr("&Upload"), treeView_);
    actionUpload->setObjectName("Upload");
    actionUpload->setStatusTip(tr("Upload file to your inventory"));
    connect(actionUpload, SIGNAL(triggered()), this, SLOT(Upload()));
    treeView_->addAction(actionUpload);

    InventoryAction *actionDownload = new InventoryAction(tr("&Download"), treeView_);
    actionDownload->setObjectName("Download");
    actionDownload->setStatusTip(tr("Download assets to your hard drive"));
    connect(actionDownload, SIGNAL(triggered()), this, SLOT(Download()));
    treeView_->addAction(actionDownload);

    // Add separator
    InventoryAction *actionSeparator = new InventoryAction(treeView_);
    actionSeparator->setSeparator(true);
    treeView_->addAction(actionSeparator);

    // Inventory item actions.
    InventoryAction  *actionDelete = new InventoryAction(tr("&Delete"), treeView_);
    actionDelete->setObjectName("Delete");
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionDelete->setStatusTip(tr("Delete this item"));
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(DeleteItem()));
    treeView_->addAction(actionDelete);

    InventoryAction *actionRename = new InventoryAction(tr("&Rename"), treeView_);
    actionRename->setObjectName("Rename");
    //actionRename_->setShortcuts();
    actionRename->setStatusTip(tr("Rename this item"));
    connect(actionRename, SIGNAL(triggered()), this, SLOT(RenameItem()));
    treeView_->addAction(actionRename);

/*
    InventoryAction  *actionCut_ = new InventoryAction(tr("&Cut"), treeView_);
    actionDelete_->setShortcuts(QKeySequence::Cut);
    actionDelete_->setStatusTip(tr("Cut this item"));
    connect(actionCut_, SIGNAL(triggered()), this, SLOT(Test()));
    treeView_->addAction(actionCut_);

    InventoryAction  *actionPaste_ = new InventoryAction(tr("&Paste"), treeView_);
    actionDelete_->setShortcuts(QKeySequence::Paste);
    actionDelete_->setStatusTip(tr("Paste this item"));
    connect(actionPaste_, SIGNAL(triggered()), this, SLOT(Test()));
    treeView_->addAction(actionPaste_);
*/

    InventoryAction  *actionNewFolder = new InventoryAction(tr("&New folder"), treeView_);
    actionNewFolder->setObjectName("NewFolder");
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionNewFolder->setStatusTip(tr("Create new folder"));
    connect(actionNewFolder, SIGNAL(triggered()), this, SLOT(AddFolder()));
    treeView_->addAction(actionNewFolder);

    InventoryAction *actionOpen = new InventoryAction(tr("&Open"), treeView_);
    actionOpen->setObjectName("Open");
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionOpen->setStatusTip(tr("Open this item"));
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(OpenItem()));
    treeView_->addAction(actionOpen);

    InventoryAction  *actionProperties= new InventoryAction(tr("&Properties"), treeView_);
    actionProperties->setObjectName("Properties");
    //actionProperties_->setShortcuts(QKeySequence::Delete);
    actionProperties->setStatusTip(tr("View item properties"));
    connect(actionProperties, SIGNAL(triggered()), this, SLOT(OpenItemProperties()));
    treeView_->addAction(actionProperties);

    InventoryAction *actionCopyAssetReference = new InventoryAction(tr("&Copy asset reference"), treeView_);
    actionCopyAssetReference->setObjectName("CopyAssetReference");
    //actionDelete_->setShortcuts(QKeySequence::Delete);
    actionCopyAssetReference->setStatusTip(tr("Copies asset reference to clipboard"));
    connect(actionCopyAssetReference, SIGNAL(triggered()), this, SLOT(CopyAssetReference()));
    treeView_->addAction(actionCopyAssetReference);
}

} // namespace Inventory
